#include "arcore.h"
#include "arcoreworker.h"

#include "src/application.h"
#include "src/settings.h"

#include <QDebug>

#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include "arcore_c_api.h"

#include <QTimer>
#include <QLoggingCategory>
#include <QScreen>

#include <QMatrix4x4>

Q_LOGGING_CATEGORY(arcore, "ARCore")


ARCore* ARCore::s_pInstance = nullptr;

ARCore::ARCore(QObject *parent)
    : QObject(parent)
    , m_check_retries(50)
    , m_supported(false)
    , m_installing(false)
    , m_installed(false)
    , m_initialized(false)
    , m_worker(new ARCoreWorker())
    , m_worker_thread()
    , m_session(nullptr)
{
    qCDebug(arcore, "Create object");

    m_worker->moveToThread(&m_worker_thread);

    QObject::connect(this, &ARCore::initialized, m_worker, &ARCoreWorker::doWork);

    QObject::connect(m_worker, &ARCoreWorker::cameraMatrix, this, &ARCore::setCameraTransform);
    QObject::connect(m_worker, &ARCoreWorker::errorOccurred, this, &ARCore::onWorkerErrorOccurred);

    m_worker_thread.start();

    if( Settings::I()->val("Position.ARCore.enable_arcore").toBool() )
        QTimer::singleShot(1000, this, &ARCore::init); // Running init with delay
}

ARCore::~ARCore()
{
    qCDebug(arcore, "Destroy object");

    m_worker->stop();
    m_worker_thread.quit();
    m_worker_thread.wait();
    delete m_worker;

    if( m_session != nullptr )
        destroySession();
}

void ARCore::stop()
{
    qCDebug(arcore, "Stop worker");
    m_worker->stop();
}

void ARCore::setSupported(bool val)
{
    if( val != m_supported ) {
        m_supported = val;
        emit supportedChanged();
    }
}

void ARCore::setInstalling(bool val)
{
    if( val != m_installing ) {
        m_installing = val;
        emit installingChanged();
    }
}

void ARCore::setInstalled(bool val)
{
    if( val != m_installed ) {
        m_installed = val;
        emit installedChanged();
    }
}

void ARCore::setInitialized(bool val)
{
    if( val != m_initialized ) {
        m_initialized = val;
        emit initializedChanged();
    }
    if( m_initialized )
        emit initialized();
}

bool ARCore::init()
{
    if( isInitialized() )
        return true;

    if( isSupported() )
        return requestInstall();

    if( m_check_retries == 50 )
        qCDebug(arcore, "Init ARCore");

    QAndroidJniEnvironment qjni_env;
    ArAvailability availability;
    ArCoreApk_checkAvailability(qjni_env, QtAndroid::androidContext().object(), &availability);

    if( availability == AR_AVAILABILITY_UNKNOWN_CHECKING && m_check_retries > 0 ) {
        qCDebug(arcore, "Check again in 200ms...");
        m_check_retries--;
        QTimer::singleShot(200, this, &ARCore::init);
        return qjni_env->ExceptionCheck();
    }

    m_check_retries = 50;

    if( availability == AR_AVAILABILITY_SUPPORTED_NOT_INSTALLED ||
        availability == AR_AVAILABILITY_SUPPORTED_APK_TOO_OLD ) {
        qCDebug(arcore) << "Ok, supported, but need package install/update" << availability;
        Application::I()->notice("Requesting ARCore package install/update");
        setSupported(true);
        return requestInstall();
    } else if( availability == AR_AVAILABILITY_SUPPORTED_INSTALLED ) {
        qCDebug(arcore) << "Ok, supported and installed" << availability;
        setSupported(true);
        setInstalled(true);
        return createSession();
    } else {
        qCWarning(arcore) << "Nope, no support" << availability;
        Application::I()->notice("Unfortunately ARCore is not supported on your system");
        Application::I()->notice("Please check that your login to Google Play is active & it is updated - and restart the application");
        setSupported(false);
    }

    return qjni_env->ExceptionCheck();
}

QQuaternion ARCore::getCameraQuaternion()
{
    QMutexLocker locker(&m_camera_transform_mutex);
    return m_camera_transform.rotation();
}

QVector3D ARCore::getCameraTranslation()
{
    QMutexLocker locker(&m_camera_transform_mutex);
    return m_camera_transform.translation();
}

void ARCore::onWorkerErrorOccurred(const QString &error)
{
    qCWarning(arcore) << "Found error in worker:" << error;
    stop();
    emit errorOccurred(error);
}

void ARCore::setCameraTransform(float *camera_quattrans)
{
    {
        QMutexLocker locker(&m_camera_transform_mutex);
        m_camera_transform.setRotation(QQuaternion(camera_quattrans[3], camera_quattrans[0], camera_quattrans[1], camera_quattrans[2]));
        m_camera_transform.setTranslation(QVector3D(camera_quattrans[4], camera_quattrans[5], camera_quattrans[6]));
    }

    emit cameraTransformChanged();
}

bool ARCore::requestInstall()
{
    if( isInstalled() )
        return createSession();

    if( !isInstalling() )
        qCDebug(arcore, "Requesting install");

    QAndroidJniEnvironment qjni_env;
    ArInstallStatus install_status;

    ArStatus status = ArCoreApk_requestInstall(qjni_env, QtAndroid::androidContext().object(), isInstalling(), &install_status);
    if( status != AR_SUCCESS ) {
        qCWarning(arcore) << "Unable to install the required packages";
        return qjni_env->ExceptionCheck();
    }

    if( install_status == AR_INSTALL_STATUS_INSTALL_REQUESTED ) {
        if( !isInstalling() ) {
            QObject::connect(Application::I(), &QGuiApplication::applicationStateChanged, this, &ARCore::requestInstall);
            setInstalling(true);
            return qjni_env->ExceptionCheck();
        } else {
            if( Application::I()->applicationState() == Qt::ApplicationActive ) {
                qCWarning(arcore, "User declined installing of ARCore");
                Application::I()->warning("ARCore install was aborted by user");
            } else
                return qjni_env->ExceptionCheck();
        }
    }

    if( isInstalling() ) {
        QObject::disconnect(Application::I(), &QGuiApplication::applicationStateChanged, this, &ARCore::requestInstall);
        setInstalling(false);
    }

    if( install_status == AR_INSTALL_STATUS_INSTALLED ) {
        qCDebug(arcore, "Install completed");
        Application::I()->notice("ARCore package was installed");
        setInstalled(true);

        return createSession();
    }

    return qjni_env->ExceptionCheck();
}

bool ARCore::createSession()
{
    if( isInitialized() )
        return true;

    qCDebug(arcore, "Creating session");

    QAndroidJniEnvironment qjni_env;

    ArStatus status = ArSession_create(qjni_env, QtAndroid::androidContext().object(), &m_session);
    if( status != AR_SUCCESS ) {
        qCWarning(arcore) << "Unable create a session";
        Application::I()->warning("ARCore unable to create session");
        destroySession();
        return qjni_env->ExceptionCheck();
    }

    m_worker->setSession(&m_session);

    QRect rect = Application::I()->primaryScreen()->geometry();
    ArSession_setDisplayGeometry(m_session, ROTATION_0, rect.width(), rect.height());

    setInitialized(true);

    Application::I()->notice("ARCore successfully initialized");

    return qjni_env->ExceptionCheck();
}

void ARCore::destroySession()
{
    if( m_session == nullptr )
        return;

    ArSession_destroy(m_session);

    setInitialized(false);
}
