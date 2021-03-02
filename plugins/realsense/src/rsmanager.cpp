#include <librealsense2/h/rs_types.h>

#include "rsmanager.h"
#include "plugin.h"

#include <QMutableHashIterator>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>
#include <QEventLoop>

Q_LOGGING_CATEGORY(rsmanager, "RealSensePlugin::RSManager")

const std::string PLATFORM_CAMERA_NAME = "Platform Camera";

QString RSManager::pluginName()
{
    return m_plugin->name();
}

void RSManager::setup(RealSensePlugin *plugin)
{
    m_plugin = plugin;
    qCDebug(rsmanager) << "Setting up realsense context...";
    // Register callback for tracking which devices are currently connected
    m_ctx.set_devices_changed_callback([&](rs2::event_information& info) {
        qCDebug(rsmanager) << "Devices changed!";
        removeDevices(info);
        try {
            for( auto&& dev : info.get_new_devices() )
                addDevice(dev);
        }
        catch( const rs2::backend_error & e ) {
            qCDebug(rsmanager) << "rs2error: " << rs2_exception_type_to_string(e.get_type());
            qCDebug(rsmanager) << QString::fromStdString(e.what());
        }
        catch( const std::exception& e ) {
            qCDebug(rsmanager) << QString::fromStdString(e.what());
        }
    });

    // Query the list of connected RealSense devices
    for( auto&& dev : m_ctx.query_devices() )
        addDevice(dev);
}

void RSManager::addDevice(rs2::device& dev)
{
    QString serialNumber(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
    qCDebug(rsmanager) << "Processing found device:" << serialNumber;
    QMutexLocker locker(&m_mutex);
    if( m_connected_devices.contains(serialNumber) ) {
        qCDebug(rsmanager) << "Skipping already existing camera:" << serialNumber;
        return;
    }
    // Ignoring platform cameras (webcams, etc..)
    if( PLATFORM_CAMERA_NAME == dev.get_info(RS2_CAMERA_INFO_NAME) ) {
        qCDebug(rsmanager) << "Skipping platform camera:" << serialNumber;
        return;
    }
    m_connected_devices.insert(serialNumber, dev);

    qCDebug(rsmanager) << "Connected camera:" << dev.get_info(RS2_CAMERA_INFO_NAME) << ":";

    emit cameraConnected(serialNumber);
}

void RSManager::removeDevices(const rs2::event_information& info)
{
    qCDebug(rsmanager) << "Removing device...";
    QMutexLocker locker(&m_mutex);
    // Go over the list of devices and check if it was disconnected
    QMutableHashIterator<QString, rs2::device> i(m_connected_devices);
    while( i.hasNext() ) {
        i.next();
        rs2::device dev = i.value();
        if( info.was_removed(dev) ) {
            QString serialNumber = i.key();
            i.remove();
            emit cameraDisconnected(serialNumber);
        }
    }
}

RSDevice* RSManager::getDevice(const QString serial)
{
    for( RSDevice *dev : m_device_list ) {
        if( dev->serialNumber() != serial )
            continue;
        return dev;
    }

    RSDevice* device = new RSDevice(this, serial);
    m_device_list.append(device);

    // Connect device "new pcdata" signal to pass it to the plugin
    connect(device, &RSDevice::pointCloudDataChanged, m_plugin, &RealSensePlugin::pointCloudCaptured);

    return device;
}

void RSManager::setAvailableStreamsStatus(QVariantMap *map, VideoSourceInterface::stream_status status) const
{
    bool ok = false;
    int curr_val = (*map)["status"].toUInt(&ok);
    if( !ok )
        curr_val = -1;
    if( curr_val < status )
        (*map)["status"] = status;
}

QMap<QString, QVariantMap> RSManager::getAvailableStreams() const
{
    QMap<QString, QVariantMap> out;

    QList<VideoSourceStreamObject*> active_streams = listVideoStreams();

    for( auto dev : m_connected_devices ) {
        VideoSourceInterface::stream_status status;
        QString dev_id = dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        QVariantMap device;
        device["name"] = QString(dev.get_info(RS2_CAMERA_INFO_NAME));
        device["description"] = QString("USB:%2 FW:%3")
                .arg(dev.get_info(RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR),
                     dev.get_info(RS2_CAMERA_INFO_FIRMWARE_VERSION));
        qCDebug(rsmanager) << __func__ << "Found device:" << device["name"] << device["description"];

        QVariantMap device_childs;
        for( auto sens : dev.query_sensors() ) {
            QString sensor_name = sens.get_info(RS2_CAMERA_INFO_NAME);
            qCDebug(rsmanager) << __func__ << "  Found sensor:" << sensor_name;
            QVariantMap sensor;

            QVariantMap sensor_childs;
            for( auto sp : sens.get_stream_profiles() ) {
                QString stream_name = QString::fromStdString(sp.stream_name());

                QVariantMap stream;
                QVariantMap stream_childs;
                if( sensor_childs.contains(stream_name) ) {
                    stream = sensor_childs[stream_name].toMap();
                    stream_childs = stream["childrens"].toMap();
                }

                {
                    QString stream_format = rs2_format_to_string(sp.format());

                    QVariantMap format;
                    QVariantMap format_childs;
                    if( stream_childs.contains(stream_format) ) {
                        format = stream_childs[stream_format].toMap();
                        format_childs = format["childrens"].toMap();
                    }

                    if( sp.format() != RS2_FORMAT_RGB8 &&
                        sp.format() != RS2_FORMAT_RGBA8 &&
                        sp.format() != RS2_FORMAT_BGR8 &&
                        sp.format() != RS2_FORMAT_Z16 &&
                        sp.format() != RS2_FORMAT_Y16 &&
                        sp.format() != RS2_FORMAT_Y8 ) {
                        status = VideoSourceInterface::NOT_SUPPORTED; // Not supported by the plugin
                    } else {
                        QString stream_fps = QString::number(sp.fps());

                        QVariantMap fps;
                        QVariantMap fps_childs;
                        if( format_childs.contains(stream_fps) ) {
                            fps = format_childs[stream_fps].toMap();
                            fps_childs = fps["childrens"].toMap();
                        }
                        {
                            auto vp = sp.as<rs2::video_stream_profile>();
                            QString stream_resolution = vp ? QString("%1x%2").arg(vp.width()).arg(vp.height()) : "not video";

                            QStringList path = QStringList() << dev_id << sensor_name << stream_name << stream_format << stream_fps << stream_resolution;
                            status = VideoSourceInterface::EMPTY;
                            for( auto stream_obj : active_streams ) {
                                if( stream_obj->path() != path )
                                    continue;
                                if( stream_obj->isCapture() )
                                    status = VideoSourceInterface::CAPTURE;
                                else
                                    status = VideoSourceInterface::ACTIVE;
                            }

                            QVariantMap resolution;
                            resolution["status"] = status;
                            fps_childs[stream_resolution] = resolution;
                        }
                        fps["childrens"] = fps_childs;
                        setAvailableStreamsStatus(&fps, status);
                        format_childs[stream_fps] = fps;
                    }
                    format["childrens"] = format_childs;
                    setAvailableStreamsStatus(&format, status);
                    stream_childs[stream_format] = format;
                }
                stream["childrens"] = stream_childs;
                setAvailableStreamsStatus(&stream, status);
                sensor_childs[stream_name] = stream;

                setAvailableStreamsStatus(&sensor, status);
                setAvailableStreamsStatus(&device, status);
            }
            sensor["childrens"] = sensor_childs;
            device_childs[sensor_name] = sensor;
        }
        device["childrens"] = device_childs;
        out[dev_id] = device;
    }
    return out;
}

VideoSourceStreamObject* RSManager::getVideoStream(const QStringList path)
{
    qCDebug(rsmanager) << __func__ << "Get video stream:" << path;

    RSDevice *device = getDevice(path[0]);

    return device->connectStream(path);
}

QList<VideoSourceStreamObject*> RSManager::listVideoStreams(const QStringList path) const
{
    QList<VideoSourceStreamObject*> out;

    for( RSDevice *dev : m_device_list ) {
        if( !path.empty() && dev->serialNumber() != path[0] )
            continue;

        for( VideoSourceStreamObject* stream : dev->listStreams(path) )
            out.append(stream);
    }

    return out;
}

rs2::stream_profile RSManager::getStreamProfile(const QStringList path)
{
    if( path.length() != 6 ) {
        qCWarning(rsmanager) << __func__ << "Required 6 items in the path:" << path;
        return rs2::stream_profile();
    }

    QMutexLocker locker(&m_mutex);
    QHash<QString, rs2::device>::iterator it = m_connected_devices.find(path[0]);
    if( it == m_connected_devices.end() ) {
        qCWarning(rsmanager) << __func__ << "Unable to find connected device:" << path;
        return rs2::stream_profile();
    }

    rs2::device dev = it.value();
    for( auto sensor : dev.query_sensors() ) {
        auto profiles = sensor.get_stream_profiles();
        for( auto sp : profiles ) {
            QStringList profile_path;
            profile_path << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
            profile_path << sensor.get_info(RS2_CAMERA_INFO_NAME);
            profile_path << QString::fromStdString(sp.stream_name());
            profile_path << rs2_format_to_string(sp.format());
            profile_path << QString::number(sp.fps());
            auto vp = sp.as<rs2::video_stream_profile>();
            profile_path << (vp ? QString("%1x%2").arg(vp.width()).arg(vp.height()) : "not video");

            if( profile_path == path )
                return sp;
        }
    }

    return rs2::stream_profile();
}

int RSManager::getConnectedDevicesSize()
{
    QMutexLocker locker(&m_mutex);
    return m_connected_devices.size();
}

QString RSManager::getDeviceInfo(QString serial, rs2_camera_info field)
{
    QMutexLocker locker(&m_mutex);
    QHash<QString, rs2::device>::iterator i = m_connected_devices.find(serial);
    if( i != m_connected_devices.end() )
        return QString(i.value().get_info(field));

    return QString();
}

QSharedPointer<PointCloudData> RSManager::getStreamPointCloud(const QString device_serial)
{
    qCDebug(rsmanager) << __func__ << "Get existing point cloud for:" << device_serial;

    RSDevice *device = getDevice(device_serial);
    if( !device ) {
        qCWarning(rsmanager) << __func__ << "Unable to find device:" << device_serial;
        return nullptr;
    }

    return device->getPointCloudData();
}

void RSManager::capturePointCloudShot(const QString device_serial)
{
    qCDebug(rsmanager) << __func__ << "Get new point cloud for:" << device_serial;

    RSDevice *device = getDevice(device_serial);
    if( !device ) {
        qCWarning(rsmanager) << __func__ << "Unable to find device:" << device_serial;
        return;
    }

    device->makeShot();
}

void RSManager::capturePointCloudShotSeries(const QString device_serial, bool val)
{
    qCDebug(rsmanager) << __func__ << "Set point cloud capture to:" << device_serial;

    RSDevice *device = getDevice(device_serial);
    if( !device ) {
        qCWarning(rsmanager) << __func__ << "Unable to find device:" << device_serial;
        return;
    }

    device->setShotSeries(val);
}
