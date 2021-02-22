#ifndef RSDEVICEWORKER_H
#define RSDEVICEWORKER_H

#include <librealsense2/rs.hpp>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QImage>

class PointCloudData;
class RSDevice;

class RSDeviceWorker
    : public QObject
{
    Q_OBJECT

public:
    explicit RSDeviceWorker(rs2::pipeline *pipe, rs2::frame_queue *queue, RSDevice *device);
    void makeShot();
    void stop();
    void setPipeline(rs2::pipeline *pipe);

private:
    bool m_stopped;
    bool m_make_shot;
    QMutex m_mutex;
    rs2::pipeline *m_pipe;
    rs2::frame_queue *m_queue;

    RSDevice *m_device;

    bool m_use_disparity_filter;
    bool m_use_spatial_filter;
    bool m_use_temporal_filter;

    QImage frameToQImage(const rs2::frame &f);
    PointCloudData* toPointCloud(rs2::points points, rs2::depth_frame depth_frame, rs2::video_frame texture, size_t width = 0);

public slots:
    void doWork();

signals:
    void newPointCloudData(PointCloudData *pcdata);
    void stopped();
    void errorOccurred(const QString &error);
};
#endif // RSDEVICEWORKER_H
