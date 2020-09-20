#ifndef RSCAMERA_H
#define RSCAMERA_H

#include "rsmanager.h"
#include "rsworker.h"

#include <QThread>

#include <librealsense2/rs.hpp>

const QString DEFAULT_DEVICE("*");

class RSCamera
    : public QObject
{
    Q_OBJECT

public:
    explicit RSCamera(const QString &serialNumber=DEFAULT_DEVICE);
    ~RSCamera() override;

    void init();
    void deinit();

    void start();
    void stop();
    void makeShot();

signals:
    void started();

private:
    QString m_serial_number;
    QString m_scanning_device_serial;
    rs2::pipeline *m_pipe;
    rs2::pipeline_profile m_profile;
    rs2::frame_queue m_queue;
    rs2::config m_config;

    RSWorker m_generator;
    QThread m_generatorThread;

    void _stop();

    void onNewDepthImage(QImage image);
    void onNewPointCloud(PointCloud* pc);
    void onGeneratorErrorOccurred(const QString &error);
};

#endif // RSCAMERA_H
