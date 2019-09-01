#ifndef ARCORE_H
#define ARCORE_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <Qt3DCore/QTransform>

class ARCoreWorker;

class ARCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool supported READ isSupported WRITE setSupported NOTIFY supportedChanged)
    Q_PROPERTY(bool installing READ isInstalling WRITE setInstalling NOTIFY installingChanged)
    Q_PROPERTY(bool installed READ isInstalled WRITE setInstalled NOTIFY installedChanged)
    Q_PROPERTY(bool initialized READ isInitialized WRITE setInitialized NOTIFY initializedChanged)

public:
    inline static ARCore* I() { if( s_pInstance == nullptr ) s_pInstance = new ARCore(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    bool init();
    void stop();

    QQuaternion getCameraQuaternion();
    QVector3D getCameraTranslation();

    bool isSupported() { return m_supported; }
    bool isInstalling() { return m_installing; }
    bool isInstalled() { return m_installed; }
    bool isInitialized() { return m_initialized; }

    void onWorkerErrorOccurred(const QString &error);

signals:
    void supportedChanged();
    void installingChanged();
    void installedChanged();
    void initializedChanged();
    void initialized();

    void cameraTransformChanged();
    void errorOccurred(const QString &error);

private slots:
    void setCameraTransform(float *camera_quattrans);

private:
    static ARCore *s_pInstance;
    explicit ARCore(QObject *parent = nullptr);
    ~ARCore() override;

    enum Rotation {
        ROTATION_0 =   0x0,
        ROTATION_90 =  0x1,
        ROTATION_180 = 0x2,
        ROTATION_270 = 0x3
    };

    bool requestInstall();
    bool createSession();
    void destroySession();

    void setSupported(bool val);
    void setInstalling(bool val);
    void setInstalled(bool val);
    void setInitialized(bool val);

    quint8 m_check_retries;
    bool m_supported;
    bool m_installing;
    bool m_installed;
    bool m_initialized;

    ARCoreWorker *m_worker;
    QThread m_worker_thread;

    typedef struct ArSession_ ArSession;
    ArSession *m_session;

    QMutex m_camera_transform_mutex;
    Qt3DCore::QTransform m_camera_transform;
};

#endif // ARCORE_H
