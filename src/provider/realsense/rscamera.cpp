#include "rscamera.h"

#include <QDebug>
#include <QLoggingCategory>
#include <limits>

#include "src/camera/pointcloud.h"
#include "src/settings.h"
#include "src/application.h"

Q_LOGGING_CATEGORY(rscamera, "RSCamera")

RSCamera* RSCamera::s_pInstance = nullptr;

const std::string NO_CAMERA_MESSAGE = "No camera connected, please connect 1 or more";

const unsigned int FRAME_QUEUE_SIZE = std::numeric_limits<unsigned int>::max();

RSCamera::RSCamera(const QString &serialNumber)
    : DepthCamera(serialNumber)
    , m_pipe(nullptr)
    , m_queue(FRAME_QUEUE_SIZE)
    , m_generator(m_pipe, &m_queue, this)
{
    qCDebug(rscamera) << "Init rscamera...";
    init();

    connect(&m_cameraManager, &RSManager::cameraConnected,
            this, &RSCamera::onCameraConnected);
    connect(&m_cameraManager, &RSManager::cameraDisconnected,
            this, &RSCamera::onCameraDisconnected);
    m_cameraManager.setup();

    m_generator.moveToThread(&m_generatorThread);

    connect(this, &RSCamera::started, &m_generator, &RSWorker::doWork);

    connect(&m_generator, &RSWorker::newDepthImage,
            this, &RSCamera::onNewDepthImage,
            Qt::ConnectionType::QueuedConnection);

    connect(&m_generator, &RSWorker::newPointCloud,
            this, &RSCamera::onNewPointCloud,
            Qt::ConnectionType::QueuedConnection);

    connect(&m_generator, &RSWorker::stopped, this, &RSCamera::_stop);

    connect(&m_generator, &RSWorker::errorOccurred, this, &RSCamera::onGeneratorErrorOccurred);

    connect(&m_generator, &RSWorker::streamFPS, this, &RSCamera::setStreamFPS);
    connect(&m_generator, &RSWorker::streamFWT, this, &RSCamera::setStreamFWT);
    connect(&m_generator, &RSWorker::streamFPT, this, &RSCamera::setStreamFPT);

    m_generatorThread.start();
    qCDebug(rscamera) << "Init rscamera done";
}

RSCamera::~RSCamera()
{
    stop();
    m_generatorThread.quit();
    m_generatorThread.wait();
    deinit();
}

void RSCamera::start()
{
    if( getIsStreaming() )
        return;

    qCDebug(rscamera) << "start streaming";

    if( m_pipe != nullptr ) {
        setIsStreaming(true);

        qCDebug(rscamera) << "start pipe";
        try {
            m_profile = m_pipe->start(m_config);
        } catch( rs2::error e ) {
            qCWarning(rscamera) << "Unable to start pipe with the current configuration:" << e.what();
            qCWarning(rscamera) << "Disabling the color stream and retry";
            m_config.disable_stream(RS2_STREAM_COLOR);
            try {
                m_profile = m_pipe->start(m_config);
            } catch( rs2::error e ) {
                Application::I()->error("Unable to start pipeline for this camera. Please report to developer");
                setIsStreaming(false);
                return;
            }
        }
        qCDebug(rscamera) << "get serial";
        m_scanningDeviceSerial = m_profile.get_device().get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        emit started();
    }
}

void RSCamera::stop()
{
    if( !getIsStreaming() )
        return;

    qCDebug(rscamera) << "stop generator";
    m_generator.stop();
}

void RSCamera::_stop()
{
    if( m_pipe != nullptr ) {
        qCDebug(rscamera) << "stop pipe";
        m_pipe->stop();
        qCDebug(rscamera) << "after pipe stop";
        setIsScanning(false);
        setIsStreaming(false);
    }
}

void RSCamera::makeShot()
{
    // TODO: move to settings
    /*try {
        qCDebug(rscamera) << "set laser max";
        auto depth_sensor = m_profile.get_device().first<rs2::depth_sensor>();
        if( depth_sensor.supports(RS2_OPTION_LASER_POWER) )
        {
            // Query min and max values:
            auto range = depth_sensor.get_option_range(RS2_OPTION_LASER_POWER);
            qCDebug(rscamera) << "Current laser power: " << depth_sensor.get_option(RS2_OPTION_LASER_POWER);
            qCDebug(rscamera) << "range.step is" << range.step;
            if( depth_sensor.get_option(RS2_OPTION_LASER_POWER) < 299.0f ) {
                qCDebug(rscamera) << "Set laser to" << range.max -range.step;
                depth_sensor.set_option(RS2_OPTION_LASER_POWER, 300.0f); // Set max power
            }
        }
    } catch( rs2::invalid_value_error e ) {
        qCWarning(rscamera) << "Found invalid value during set max power" << e.get_failed_function().c_str() << e.what();
    }*/

    if( getIsStreaming() && ! getIsScanning() )
        m_generator.makeShot();
}

void RSCamera::init()
{
    if( m_pipe == nullptr )
        m_pipe = new rs2::pipeline();
    m_generator.setPipeline(m_pipe);
}

void RSCamera::deinit()
{
    if( m_pipe != nullptr )
        delete m_pipe;
}

void RSCamera::onNewDepthImage(QImage image)
{
    emit newDepthImage(image);
}

void RSCamera::onNewPointCloud(PointCloud *pc)
{
    qCDebug(rscamera) << "Adding new pointcloud to the list";
    addPointCloud(pc);
}

void RSCamera::onCameraConnected(const QString &serialNumber)
{
    qCDebug(rscamera) << "Connected: " << serialNumber << m_serialNumber;
    QString usb_version = m_cameraManager.getCameraInfo(serialNumber, RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR);
    setConnectionType("USB "+usb_version);
    qCDebug(rscamera) << "USB version:" << usb_version;

    qCDebug(rscamera) << "Setting stream parameters";
    m_config.enable_device(serialNumber.toStdString());
    m_config.disable_all_streams();

    // Checking usb speed
    if( usb_version.toFloat() < 3.0f ) {
        int fr = 6;
        // D455 is not support the 6fps output, so this improper fix for now
        if( m_cameraManager.getCameraInfo(serialNumber, RS2_CAMERA_INFO_NAME).contains("D455") )
            fr = 5;
        m_config.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, fr);
        if( Settings::I()->value("Camera.Streams.enable_color_stream").toBool() )
            m_config.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8, fr);
    } else {
        m_config.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
        if( Settings::I()->value("Camera.Streams.enable_color_stream").toBool() )
            m_config.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8, 30);
    }
    setIsConnected(true);
}

void RSCamera::onCameraDisconnected(const QString &serialNumber)
{
    qCDebug(rscamera) << "Disconnected: " << serialNumber;
    if( m_serialNumber == DEFAULT_DEVICE && serialNumber == m_scanningDeviceSerial ) {
        if( getIsStreaming() )
            stop();
        if( m_cameraManager.getConnectedDevicesSize() <= 0 )
            setIsConnected(false);
    } else if( serialNumber == m_serialNumber ) {
        if( getIsStreaming() )
            stop();

        setIsConnected(false);
    }
    setConnectionType("");
}

void RSCamera::onGeneratorErrorOccurred(const QString &error)
{
    qCDebug(rscamera) << error;
    stop();
    setIsStreaming(false);
    setIsScanning(false);
    emit errorOccurred(error);
}
