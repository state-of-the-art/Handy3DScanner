#ifndef RSDEVICE_H
#define RSDEVICE_H

#include <QThread>
#include <QSharedPointer>

#include <librealsense2/rs.hpp>

#include "rsdeviceworker.h"
#include "VideoSourceStreamObject.h"
#include "PointCloudData.h"

class RSManager;
class RSDevice
    : public QObject
{
    Q_OBJECT

public:
    explicit RSDevice(RSManager *rsmanager, const QString serial_number);
    ~RSDevice() override;

    friend class RSDeviceWorker;

    const QString serialNumber() const { return m_serial_number; };

    VideoSourceStreamObject* getStream(const QStringList path);
    VideoSourceStreamObject* connectStream(const QStringList path);
    QList<VideoSourceStreamObject*> listStreams(const QStringList path = QStringList());

    bool getIsConnected() const { return m_isconnected; };
    bool getIsStarted() const { return m_isstarted; };
    bool getIsStreaming() const { return m_isstreaming; };

    void init();
    void deinit();

    void start();
    void stop();
    void restart();
    void makeShot();
    void setShotSeries(bool val);

    QList<VideoSourceStreamObject*> getVideoStreams() { return m_video_streams; };
    QSharedPointer<PointCloudData> getPointCloudData() { return m_pcdata; };

signals:
    void connected();
    void disconnected();
    void streaming();
    void notstreaming();
    void started();
    void stopped();
    void pointCloudDataChanged(const QString device_id, QSharedPointer<PointCloudData> pcdata);

public slots:
    void onPointCloudData(PointCloudData *pcdata);

protected:
    void setIsConnected(const bool val);
    void setIsStarted(const bool val);
    void setIsStreaming(const bool val);

private:
    RSManager *m_rsmanager;
    QString m_serial_number;
    rs2::pipeline *m_pipe;
    rs2::frame_queue m_queue;
    rs2::config m_config;

    QList<VideoSourceStreamObject*> m_video_streams;

    RSDeviceWorker m_generator;
    QThread m_generator_thread;

    bool m_isconnected;
    bool m_isstreaming;
    bool m_isstarted;

    QSharedPointer<PointCloudData> m_pcdata;

    void _stop();
};

#endif // RSDEVICE_H
