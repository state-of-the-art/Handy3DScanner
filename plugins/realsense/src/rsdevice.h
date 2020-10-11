#ifndef RSDEVICE_H
#define RSDEVICE_H

#include <QThread>

#include <librealsense2/rs.hpp>

#include "rsdeviceworker.h"
#include "VideoSourceStreamObject.h"

class RSManager;
class RSDevice
    : public QObject
{
    Q_OBJECT

public:
    explicit RSDevice(RSManager *rsmanager, const QString serial_number);
    ~RSDevice() override;

    const QString serialNumber() const { return m_serial_number; };

    VideoSourceStreamObject* connectStream(const QString name);

    bool getIsConnected() const { return m_isconnected; };
    bool getIsStreaming() const { return m_isstreaming; };

    void init();
    void deinit();

    void start();
    void stop();
    void makeShot();

    QList<VideoSourceStreamObject*> getVideoStreams() { return m_video_streams; };

signals:
    void started();

protected:
    void setIsConnected(const bool is_connected) { m_isconnected = is_connected; };
    void setIsStreaming(const bool is_streaming) { m_isstreaming = is_streaming; };

private:
    RSManager *m_rsmanager;
    QString m_serial_number;
    rs2::pipeline *m_pipe;
    rs2::pipeline_profile m_profile;
    rs2::frame_queue m_queue;
    rs2::config m_config;

    QList<VideoSourceStreamObject*> m_video_streams;

    RSDeviceWorker m_generator;
    QThread m_generator_thread;

    bool m_isconnected;
    bool m_isstreaming;

    void _stop();

    void onNewDepthImage(QImage image);
    /*void onNewPointCloud(PointCloud* pc);
    void onGeneratorErrorOccurred(const QString &error);*/
};

#endif // RSDEVICE_H
