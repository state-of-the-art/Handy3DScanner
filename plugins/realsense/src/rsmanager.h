#ifndef RSMANAGER_H
#define RSMANAGER_H

#include <librealsense2/rs.hpp>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QHash>

//#include "rscamera.h"

class RSManager
    : public QObject
{
    Q_OBJECT

public:
    RSManager();
    void setup();
    int getConnectedDevicesSize();

    QString getCameraInfo(QString serial, rs2_camera_info field);

    QStringList getAvailableStreams() const;

private:
    rs2::context m_ctx;
    QMutex m_mutex;
    QHash<QString, rs2::device> m_connected_devices;

    void addDevice(rs2::device& dev);
    void removeDevices(const rs2::event_information& info);

    QString streamName(rs2::sensor &sensor, rs2::stream_profile &profile) const;

    //QList<RSCamera*> m_camera_list;

signals:
    void cameraConnected(const QString &serialNumber);
    void cameraDisconnected(const QString &serialNumber);
};

#endif // RSMANAGER_H
