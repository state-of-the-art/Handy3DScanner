#include <librealsense2/h/rs_types.h>

#include "rsmanager.h"

#include <QMutableHashIterator>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(rsmanager, "RSManager")

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
    qCDebug(rsmanager) << "Processing found device: " << serialNumber;
    QMutexLocker locker(&m_mutex);
    if( m_connectedDevices.contains(serialNumber) ) {
        qCDebug(rsmanager) << "Skipping already existing camera:" << serialNumber;
        return;
    }
    // Ignoring platform cameras (webcams, etc..)
    if( PLATFORM_CAMERA_NAME == dev.get_info(RS2_CAMERA_INFO_NAME) ) {
        qCDebug(rsmanager) << "Skipping platform camera:" << serialNumber;
        return;
    }
    m_connectedDevices.insert(serialNumber, dev);

    qCDebug(rsmanager) << "Connected camera: " << dev.get_info(RS2_CAMERA_INFO_NAME) << ":";

    emit cameraConnected(serialNumber);
}

void RSManager::removeDevices(const rs2::event_information& info)
{
    qCDebug(rsmanager) << "Removing device...";
    QMutexLocker locker(&m_mutex);
    // Go over the list of devices and check if it was disconnected
    QMutableHashIterator<QString, rs2::device> i(m_connectedDevices);
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

int RSManager::getConnectedDevicesSize()
{
    QMutexLocker locker(&m_mutex);
    return m_connectedDevices.size();
}

QString RSManager::getCameraInfo(QString serial, rs2_camera_info field)
{
    QMutexLocker locker(&m_mutex);
    QHash<QString, rs2::device>::const_iterator i = m_connectedDevices.find(serial);
    if( i != m_connectedDevices.end() )
        return QString(i.value().get_info(field));

    return QString();
}
