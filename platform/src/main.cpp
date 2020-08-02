#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>
#include <QtDebug>

#include <QFontDatabase>

#include "plugins.h"

#include "settings.h"

#include "application.h"

#include "videoplayer.h"
#include "camera/depthcamera.h"
//#include "provider/realsense/rscamera.h"
#include "camera/cameraposition.h"
#include "scenefile.h"

#include <QOpenGLContext>

#ifdef ANDROID
//#include "provider/arcore/arcore.h"
#include "androidwrapper.h"
#else
#include "linuxwrapper.h"
#endif

bool OGLSupports(int major, int minor)
{
    QOpenGLContext ctx;
    QSurfaceFormat fmt;
    fmt.setVersion(major, minor);
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);

    ctx.setFormat(fmt);
    ctx.create();
    if (!ctx.isValid())
        return false;
    int ctxMajor = ctx.format().majorVersion();
    int ctxMinor = ctx.format().minorVersion();
    bool isGles = (ctx.format().renderableType() == QSurfaceFormat::OpenGLES);

    if (isGles != true) return false;
    if (ctxMajor < major) return false;
    if (ctxMajor == major && ctxMinor < minor)
        return false;
    return true;
}

int main(int argc, char *argv[])
{
    qDebug("[h3ds] Init v%s", PROJECT_VERSION);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("State Of The Art");
    QCoreApplication::setOrganizationDomain("state-of-the-art.io");
    QCoreApplication::setApplicationName("Handy 3D Scanner");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    Application::create(argc, argv);

    qDebug() << "Support for GLES 2.0 "<<( OGLSupports(2,0) ? "yes" : "no");

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);

    // Request OpenGL ES 3 context
    if( OGLSupports(3,2) ) {
        qDebug("Requesting 3.2 GLES context");
        fmt.setVersion(3, 2);
    } else if( OGLSupports(3,1) ) {
        qDebug("Requesting 3.1 GLES context");
        fmt.setVersion(3, 1);
    } else if( OGLSupports(3,0) ) {
        qWarning("Error: This system does not support OpenGL Compute Shaders!");
        qDebug("Requesting 3.0 GLES context");
        fmt.setVersion(3, 0);
    } else
        qWarning("Error: This system does not support OpenGL 3!");

    QSurfaceFormat::setDefaultFormat(fmt);

    QQmlApplicationEngine engine;

    VideoPlayer::declareQML();
    DepthCamera::declareQML();
    qDebug("[h3ds] Init plugins...");
    engine.rootContext()->setContextProperty("plugins", Plugins::I());

    qDebug("[h3ds] Init settings...");
    engine.rootContext()->setContextProperty("cfg", Settings::I());

    qDebug("[h3ds] Init application...");
    engine.rootContext()->setContextProperty("app", Application::I());

    qDebug("[h3ds] Init camera...");
    // TODO: Restore functional
    //engine.rootContext()->setContextProperty("camera", RSCamera::I());
    engine.rootContext()->setContextProperty("cameraPos", CameraPosition::I());

    engine.rootContext()->setContextProperty("sceneFile", SceneFile::I());

    // TODO: H3DS-27 Create universal built-in file/dir picker
#ifdef ANDROID
    // TODO: Restore functional
    //engine.rootContext()->setContextProperty("arcore", ARCore::I());
    engine.rootContext()->setContextProperty("fileOpener", AndroidWrapper::I());
#else
    engine.rootContext()->setContextProperty("fileOpener", LinuxWrapper::I());
#endif

    QFontDatabase fontDatabase;
    if( fontDatabase.addApplicationFont(":/qml/fonts/icons.ttf") == -1 )
        qWarning() << "Failed to load icons.ttf";

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if( engine.rootObjects().isEmpty() )
        return -1;

    qDebug("[h3ds] Init done, starting");

    int ret = Application::I()->exec();

#ifdef ANDROID
    // TODO: Restore functional
    //ARCore::destroyI();
    AndroidWrapper::destroyI();
#else
    LinuxWrapper::destroyI();
#endif
    SceneFile::destroyI();
    CameraPosition::destroyI();
    // TODO: Restore functional
    //RSCamera::destroyI();

    //Application::destroyI();
    Settings::destroyI();

    return ret;
}
