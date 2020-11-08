#include <librealsense2/h/rs_types.h>

#include "rsmanager.h"

#include <QMutableHashIterator>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(rsmanager, "RealSensePlugin::RSManager")

const std::string PLATFORM_CAMERA_NAME = "Platform Camera";

RSManager::RSManager()
{
}

void RSManager::setup()
{
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
    return device;
}

QMap<QString, QString> RSManager::getAvailableStreams(const QStringList path) const
{
    QMap<QString, QString> out;
    for( auto dev : m_connected_devices ) {
        QString dev_id = dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        qCDebug(rsmanager) << __func__ << "Check device:" << dev.get_info(RS2_CAMERA_INFO_NAME);
        if( path.length() == 0 ) {
            out[dev_id] = QString("%1 (USB:%2 FW:%3)")
                    .arg(dev.get_info(RS2_CAMERA_INFO_NAME))
                    .arg(dev.get_info(RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR))
                    .arg(dev.get_info(RS2_CAMERA_INFO_FIRMWARE_VERSION));
            continue;
        }
        else if( path[0] != dev_id )
            continue;

        for( auto sensor : dev.query_sensors() ) {
            QString sensor_name = sensor.get_info(RS2_CAMERA_INFO_NAME);
            qCDebug(rsmanager) << __func__ << "  Check sensor:" << sensor_name;
            if( path.length() == 1 ) {
                out[sensor_name] = sensor_name;
                continue;
            }
            else if( path[1] != sensor_name )
                continue;

            for( auto sp : sensor.get_stream_profiles() ) {
                QString stream_name = QString::fromStdString(sp.stream_name());
                if( path.length() == 2 ) {
                    out[stream_name] = stream_name;
                    continue;
                }
                else if( path[2] != stream_name )
                    continue;

                QString stream_format = rs2_format_to_string(sp.format());
                if( path.length() == 3 ) {
                    out[stream_format] = stream_format;
                    continue;
                }
                else if( path[3] != stream_format )
                    continue;

                QString stream_fps = QString::number(sp.fps());
                if( path.length() == 4 ) {
                    out[stream_fps] = stream_fps;
                    continue;
                }
                else if( path[4] != stream_fps )
                    continue;

                auto vp = sp.as<rs2::video_stream_profile>();
                QString stream_resolution = vp ? QString("%1x%2").arg(vp.width()).arg(vp.height()) : "not video";
                if( path.length() == 5 ) {
                    out[stream_resolution] = stream_resolution;
                    continue;
                }
                return out;
            }
        }
    }
    return out;
}

VideoSourceStreamObject* RSManager::getVideoStream(const QStringList path)
{
    qCDebug(rsmanager) << __func__ << "Get video stream:" << path;

    RSDevice *device = getDevice(path[0]);

    return device->connectStream(path);
}

rs2::stream_profile RSManager::getStreamProfile(const QStringList path)
{
    if( path.length() != 6 ) {
        qCWarning(rsmanager) << __func__ << "Required 6 items in the path:" << path;
        return rs2::stream_profile();
    }

    QMutexLocker locker(&m_mutex);
    QHash<QString, rs2::device>::const_iterator it = m_connected_devices.find(path[0]);
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
    QHash<QString, rs2::device>::const_iterator i = m_connected_devices.find(serial);
    if( i != m_connected_devices.end() )
        return QString(i.value().get_info(field));

    return QString();
}
