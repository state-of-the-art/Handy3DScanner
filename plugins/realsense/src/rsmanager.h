#ifndef RSMANAGER_H
#define RSMANAGER_H

#include <librealsense2/rs.hpp>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QHash>
#include <QVariant>
#include <QMap>

#include "PointCloudData.h"
#include "VideoSource/VideoSourceStream.h"
#include "plugins/VideoSourceInterface.h"
#include "rsdevice.h"

class RSManager
    : public QObject
{
    Q_OBJECT

public:
    RSManager() {}
    void setup();
    int getConnectedDevicesSize();

    QString getDeviceInfo(QString serial, rs2_camera_info field);

    QMap<QString, QVariantMap> getAvailableStreams() const;
    VideoSourceStreamObject* getVideoStream(const QStringList path);
    QList<VideoSourceStreamObject*> listVideoStreams(const QStringList path = QStringList()) const;
    rs2::stream_profile getStreamProfile(const QStringList path);

    QSharedPointer<PointCloudData> getStreamPointCloud(const QString device_serial);

signals:
    void cameraConnected(const QString &serialNumber);
    void cameraDisconnected(const QString &serialNumber);

private:
    rs2::context m_ctx;
    QMutex m_mutex;
    QList<RSDevice*> m_device_list;
    QHash<QString, rs2::device> m_connected_devices;

    void addDevice(rs2::device& dev);
    void removeDevices(const rs2::event_information& info);

    RSDevice* getDevice(const QString serial);

    void setAvailableStreamsStatus(QVariantMap *map, VideoSourceInterface::stream_status status) const;
};

#endif // RSMANAGER_H
