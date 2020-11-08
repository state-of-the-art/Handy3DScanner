#include "rsdevice.h"

#include <QDebug>
#include <QLoggingCategory>
#include <limits>

#include "rsmanager.h"

//#include "camera/pointcloud.h"
//#include "settings.h"

Q_LOGGING_CATEGORY(rsdevice, "RealSensePlugin::RSDevice")

const unsigned int FRAME_QUEUE_SIZE = std::numeric_limits<unsigned int>::max();

RSDevice::RSDevice(RSManager *rsmanager, const QString serial_number)
    : QObject()
    , m_rsmanager(rsmanager)
    , m_serial_number(serial_number)
    , m_pipe(nullptr)
    , m_queue(FRAME_QUEUE_SIZE)
    , m_generator(m_pipe, &m_queue, this)
    , m_isconnected(false)
    , m_isstreaming(false)
    , m_isstarted(false)
{
    qCDebug(rsdevice) << "Create new" << m_serial_number;
    init();

    m_generator.moveToThread(&m_generator_thread);

    connect(this, &RSDevice::started, &m_generator, &RSDeviceWorker::doWork);

    connect(&m_generator, &RSDeviceWorker::newDepthImage,
            this, &RSDevice::onNewDepthImage,
            Qt::ConnectionType::QueuedConnection);

    /*connect(&m_generator, &RSDeviceWorker::newPointCloud,
            this, &RSDevice::onNewPointCloud,
            Qt::ConnectionType::QueuedConnection);*/

    connect(&m_generator, &RSDeviceWorker::stopped, this, &RSDevice::_stop);

    //connect(&m_generator, &RSDeviceWorker::errorOccurred, this, &RSDevice::onGeneratorErrorOccurred);

    /*connect(&m_generator, &RSDeviceWorker::streamFPS, this, &RSDevice::setStreamFPS);
    connect(&m_generator, &RSDeviceWorker::streamFWT, this, &RSDevice::setStreamFWT);
    connect(&m_generator, &RSDeviceWorker::streamFPT, this, &RSDevice::setStreamFPT);*/

    m_generator_thread.start();

    qCDebug(rsdevice) << "Created" << m_serial_number;
}

RSDevice::~RSDevice()
{
    stop();
    m_generator_thread.quit();
    m_generator_thread.wait();
    deinit();
}

VideoSourceStreamObject* RSDevice::connectStream(const QStringList path)
{
    qCDebug(rsdevice) << __func__ << "Setting stream parameters for path" << path;

    // Check if stream is already enabled
    for( VideoSourceStreamObject* stream : m_video_streams ) {
        if( stream->path() == path )
            return stream;
    }

    rs2::stream_profile sp = m_rsmanager->getStreamProfile(path);

    // Check if there is a stream from the same sensor
    // rs2 can't stream multiple formats from the same sensor
    for( VideoSourceStreamObject* stream : m_video_streams ) {
        if( stream->rs2_stream_type == sp.stream_type() ) {
            m_video_streams.removeOne(stream);
            // TODO: Not quite right to just delete the object without mutex
            // because the others could use it... Use shared_ptr here for automatic destroy
            //delete stream;
            break;
        }
    }

    auto vp = sp.as<rs2::video_stream_profile>();
    m_config.enable_stream(sp.stream_type(), vp.width(), vp.height(), sp.format(), sp.fps());

    QStringList description;
    description << QString("%1 (USB:%2 FW:%3)")
            .arg(m_rsmanager->getDeviceInfo(m_serial_number, RS2_CAMERA_INFO_NAME))
            .arg(m_rsmanager->getDeviceInfo(m_serial_number, RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR))
            .arg(m_rsmanager->getDeviceInfo(m_serial_number, RS2_CAMERA_INFO_FIRMWARE_VERSION));
    VideoSourceStreamObject *stream = new VideoSourceStreamObject(path, description, sp.stream_type());
    m_video_streams.append(stream);

    restart();

    return stream;
}

void RSDevice::start()
{
    if( getIsStarted() )
        return;

    // Restart only once
    disconnect(this, &RSDevice::stopped, this, &RSDevice::start);

    qCDebug(rsdevice) << "Start streaming" << m_serial_number;

    if( m_pipe != nullptr ) {
        try {
            m_pipe->start(m_config);
            setIsStarted(true);
        } catch( rs2::error e ) {
            // TODO: when there is not enough bandwidth
            qCWarning(rsdevice) << "Unable to start pipe with the current configuration:" << e.what();
            qCWarning(rsdevice) << "Disabling the color stream and retry";
            //m_config.disable_stream(RS2_STREAM_COLOR);
            //m_profile = m_pipe->start(m_config);
        }
    } else
        qCWarning(rsdevice) << "The pipe is already here which should not happen for" << m_serial_number;
}

void RSDevice::restart()
{
    connect(this, &RSDevice::stopped, this, &RSDevice::start);
    stop();
    start();
}

void RSDevice::stop()
{
    if( !getIsStarted() )
        return;

    qCDebug(rsdevice) << "Stop generator for" << m_serial_number;
    m_generator.stop();

}

void RSDevice::_stop()
{
    if( m_pipe != nullptr ) {
        qCDebug(rsdevice) << "Stop pipe for" << m_serial_number;
        m_pipe->stop();
        setIsStarted(false);
    }
}

void RSDevice::setIsConnected(const bool val)
{
    if( m_isconnected != val ) {
        m_isconnected = val;
        emit m_isconnected ? connected() : disconnected();
    }
}

void RSDevice::setIsStreaming(const bool val)
{
    if( m_isstreaming != val ) {
        m_isstreaming = val;
        emit m_isstreaming ? streaming() : notstreaming();
    }
}

void RSDevice::setIsStarted(const bool val)
{
    if( m_isstarted != val ) {
        m_isstarted = val;
        emit m_isstarted ? started() : stopped();
    }
}

void RSDevice::makeShot()
{
    // TODO: move to settings
    /*try {
        qCDebug(rsdevice) << "set laser max";
        auto depth_sensor = m_profile.get_device().first<rs2::depth_sensor>();
        if( depth_sensor.supports(RS2_OPTION_LASER_POWER) )
        {
            // Query min and max values:
            auto range = depth_sensor.get_option_range(RS2_OPTION_LASER_POWER);
            qCDebug(rsdevice) << "Current laser power: " << depth_sensor.get_option(RS2_OPTION_LASER_POWER);
            qCDebug(rsdevice) << "range.step is" << range.step;
            if( depth_sensor.get_option(RS2_OPTION_LASER_POWER) < 299.0f ) {
                qCDebug(rsdevice) << "Set laser to" << range.max -range.step;
                depth_sensor.set_option(RS2_OPTION_LASER_POWER, 300.0f); // Set max power
            }
        }
    } catch( rs2::invalid_value_error e ) {
        qCWarning(rsdevice) << "Found invalid value during set max power" << e.get_failed_function().c_str() << e.what();
    }*/

    if( getIsStreaming() )
        m_generator.makeShot();
}

void RSDevice::init()
{
    qCDebug(rsdevice) << "Init device" << m_serial_number;

    m_pipe = new rs2::pipeline();
    m_generator.setPipeline(m_pipe);

    m_config.enable_device(m_serial_number.toStdString());
    m_config.disable_all_streams();
}

void RSDevice::deinit()
{
    qCDebug(rsdevice) << "Deinitializing:" << m_serial_number;

    stop();
    setIsConnected(false);

    if( m_pipe != nullptr )
        delete m_pipe;
}

void RSDevice::onNewDepthImage(QImage image)
{
    //emit newDepthImage(image);
}

/*void RSDevice::onNewPointCloud(PointCloud *pc)
{
    qCDebug(rsdevice) << "Adding new pointcloud to the list";
    addPointCloud(pc);
}*/

/*void RSDevice::onGeneratorErrorOccurred(const QString &error)
{
    qCDebug(rsdevice) << error;
    stop();
    setIsStreaming(false);
    setIsScanning(false);
    emit errorOccurred(error);
}*/
