#ifndef RSCAMERA_H
#define RSCAMERA_H

#include "src/camera/depthcamera.h"
#include "rsmanager.h"
#include "rsworker.h"

#include <QThread>

#include <librealsense2/rs.hpp>

const QString DEFAULT_DEVICE("*");

class RSCamera
    : public DepthCamera
{
    Q_OBJECT

public:
    inline static RSCamera* I() { if( s_pInstance == nullptr ) s_pInstance = new RSCamera(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    void init();
    void deinit();

    void start() override;
    void stop() override;
    void makeShot() override;

signals:
    void started();

private:
    static RSCamera *s_pInstance;
    explicit RSCamera(const QString &serialNumber=DEFAULT_DEVICE);
    ~RSCamera() override;

    QString m_scanningDeviceSerial;
    rs2::pipeline *m_pipe;
    rs2::pipeline_profile m_profile;
    rs2::frame_queue m_queue;
    rs2::config m_config;

    RSManager m_cameraManager;

    RSWorker m_generator;
    QThread m_generatorThread;

    void _stop();

    void onNewDepthImage(QImage image);
    void onNewPointCloud(PointCloud* pc);
    void onGeneratorErrorOccurred(const QString &error);
    void onCameraConnected(const QString &serialNumber);
    void onCameraDisconnected(const QString &serialNumber);
};

#endif // RSCAMERA_H
