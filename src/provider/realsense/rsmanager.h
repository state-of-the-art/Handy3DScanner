#ifndef RSMANAGER_H
#define RSMANAGER_H

#include <librealsense2/rs.hpp>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QHash>


class RSManager
    : public QObject
{
    Q_OBJECT

public:
    RSManager();
    void setup();
    int getConnectedDevicesSize();
    QString getCameraInfo(QString serial, rs2_camera_info field);

private:
    rs2::context m_ctx;
    QMutex m_mutex;
    QHash<QString, rs2::device> m_connectedDevices;
    void addDevice(rs2::device& dev);
    void removeDevices(const rs2::event_information& info);

signals:
    void cameraConnected(const QString &serialNumber);
    void cameraDisconnected(const QString &serialNumber);
};

#endif // RSMANAGER_H
