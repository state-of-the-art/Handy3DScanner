#ifndef RSMANAGER_H
#define RSMANAGER_H

#include <librealsense2/rs.hpp>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QHash>

#include "VideoSource/VideoSourceStream.h"
#include "rsdevice.h"

class RSManager
    : public QObject
{
    Q_OBJECT

public:
    RSManager();
    void setup();
    int getConnectedDevicesSize();

    QString getDeviceInfo(QString serial, rs2_camera_info field);

    QStringList getAvailableStreams() const;
    VideoSourceStreamObject* getVideoStream(const QString path);
    rs2::stream_profile getStreamProfile(const QString serial, const QString name);

private:
    rs2::context m_ctx;
    QMutex m_mutex;
    QHash<QString, rs2::device> m_connected_devices;

    void addDevice(rs2::device& dev);
    void removeDevices(const rs2::event_information& info);

    RSDevice* getDevice(const QString serial);

    QString streamPath(rs2::sensor &sensor, rs2::stream_profile &profile) const;

    QList<RSDevice*> m_device_list;

signals:
    void cameraConnected(const QString &serialNumber);
    void cameraDisconnected(const QString &serialNumber);
};

#endif // RSMANAGER_H
