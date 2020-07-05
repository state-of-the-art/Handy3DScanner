#ifndef RSWORKER_H
#define RSWORKER_H

#include <librealsense2/rs.hpp>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QImage>

class PointCloud;

class RSWorker
    : public QObject
{
    Q_OBJECT

public:
    explicit RSWorker(rs2::pipeline *pipe, rs2::frame_queue *queue, QObject *parent);
    void makeShot();
    void stop();
    void setPipeline(rs2::pipeline *pipe);

private:
    bool m_stopped;
    bool m_make_shot;
    QMutex m_mutex;
    rs2::pipeline *m_pipe;
    rs2::frame_queue *m_queue;

    QObject *m_parent;

    bool m_use_disparity_filter;
    bool m_use_spatial_filter;
    bool m_use_temporal_filter;

    QImage frameToQImage(const rs2::frame &f);
    PointCloud* toPointCloud(rs2::points points, rs2::depth_frame depth_frame, rs2::video_frame texture, size_t width = 0);

public slots:
    void doWork();

signals:
    void newDepthImage(QImage image);
    void newColorImage(QImage image);
    void newPointCloud(PointCloud* pc);
    void stopped();
    void errorOccurred(const QString &error);

    void streamFPS(const qreal fps); // Frames per second
    void streamFWT(const qreal fwt); // Frame wait time (sec)
    void streamFPT(const qreal fpt); // Frame processing time (sec)
};
#endif // RSWORKER_H
