#include "plugin.h"

#include "rsmanager.h"

#include <QVariant>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(plugin, "RealSensePlugin")

#ifdef ANDROID
#    include <jni.h>
#    include <QtAndroidExtras>
#    include <QAndroidJniObject>

RealSensePlugin* RealSensePlugin::s_pInstance = nullptr;

// Two functions from librelasense used to add/remove device
extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_DeviceWatcher_nAddUsbDevice(JNIEnv *env, jclass type, jstring deviceName_, jint fileDescriptor);
extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_DeviceWatcher_nRemoveUsbDevice(JNIEnv *env, jclass type, jint fileDescriptor);

JNIEXPORT void JNICALL app_notice(JNIEnv *, jclass, jstring message) {
    QAndroidJniObject msg = message;
    if( RealSensePlugin::s_pInstance != nullptr )
        emit RealSensePlugin::s_pInstance->appNotice(msg.toString());
}

JNIEXPORT void JNICALL app_warning(JNIEnv *, jclass, jstring message) {
    QAndroidJniObject msg = message;
    if( RealSensePlugin::s_pInstance != nullptr )
        emit RealSensePlugin::s_pInstance->appWarning(msg.toString());
}

JNIEXPORT void JNICALL app_error(JNIEnv *, jclass, jstring message) {
    QAndroidJniObject msg = message;
    if( RealSensePlugin::s_pInstance != nullptr )
        emit RealSensePlugin::s_pInstance->appError(msg.toString());
}

#endif // ANDROID

QString RealSensePlugin::name() const
{
    return QLatin1String(plugin().categoryName());
}

QStringList RealSensePlugin::requirements() const
{
    qCDebug(plugin) << __func__;
    return QStringList();
}

bool RealSensePlugin::init()
{
    if( isInitialized() )
        return true;

    qCDebug(plugin) << __func__;
    RealSensePlugin::s_pInstance = this;

    // Setup the manager
    if( !m_rsmanager )
        m_rsmanager = new RSManager();
    m_rsmanager->setup(this);

#ifdef ANDROID
    JavaVM* vm = QAndroidJniEnvironment::javaVM();
    JNIEnv* env;
    if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ) {
        return false;
    }

    // Attach librs native functions to the RealSensePlugin class
    JNINativeMethod methods[] {{"notifyDeviceAttached", "(Ljava/lang/String;I)V", reinterpret_cast<void *>(Java_com_intel_realsense_librealsense_DeviceWatcher_nAddUsbDevice)},
                               {"notifyDeviceDetached", "(I)V", reinterpret_cast<void *>(Java_com_intel_realsense_librealsense_DeviceWatcher_nRemoveUsbDevice)},
                               {"appNotice", "(Ljava/lang/String;)V", reinterpret_cast<void *>(app_notice)},
                               {"appWarning", "(Ljava/lang/String;)V", reinterpret_cast<void *>(app_warning)},
                               {"appError", "(Ljava/lang/String;)V", reinterpret_cast<void *>(app_error)}};

    jclass activityClass = env->FindClass("io/stateoftheart/handy3dscanner/plugins/RealSensePlugin");
    if( !activityClass ) {
        qCWarning(plugin) << "Couldn't find class:" << "io/stateoftheart/handy3dscanner/plugins/RealSensePlugin";
        return false;
    }

    if( env->RegisterNatives(activityClass, methods, sizeof(methods) / sizeof(methods[0])) < 0 ) {
        qCWarning(plugin) << "Error registering JNI methods";
        return false;
    }

    env->DeleteLocalRef(activityClass);

    qCDebug(plugin) << "JNI Native methods registered";

    // Create plugin activity to listen the camera events
    QAndroidIntent serviceIntent(QtAndroid::androidActivity().object(),
                                        "io/stateoftheart/handy3dscanner/plugins/RealSensePlugin");
    QAndroidJniObject result = QtAndroid::androidActivity().callObjectMethod(
                "startService",
                "(Landroid/content/Intent;)Landroid/content/ComponentName;",
                serviceIntent.handle().object());

    qCDebug(plugin) << "Camera connect listening service activated" << result.toString();
#endif // ANDROID

    qCDebug(plugin) << "init() done";

    setInitialized(true);

    emit appNotice("RealSensePlugin initialized");

    return true;
}

bool RealSensePlugin::deinit()
{
    if( !isInitialized() )
        return true;
    qCDebug(plugin) << __func__;

#ifdef ANDROID
    JavaVM* vm = QAndroidJniEnvironment::javaVM();
    JNIEnv* env;
    if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ) {
        return false;
    }

    // Stop the plugin service
    QAndroidIntent serviceIntent(QtAndroid::androidActivity().object(),
                                 "io/stateoftheart/handy3dscanner/plugins/RealSensePlugin");

    if( ! QtAndroid::androidActivity().callMethod<jboolean>(
            "stopService", "(Landroid/content/Intent;)Z", serviceIntent.handle().object()) ) {
        qCWarning(plugin) << "Unable to deactivate camera connect listening service";
    } else
        qCDebug(plugin) << "Camera connect listening service deactivated";

    // Unbining the native librs methods
    // TODO: onDestroy is calling after "deinit() done" so we have to leave the bindings there
    /*jclass objectClass = env->FindClass("io/stateoftheart/handy3dscanner/plugins/RealSensePlugin");
    if( !objectClass ) {
        qCWarning(plugin) << "Couldn't find class:" << "io/stateoftheart/handy3dscanner/plugins/RealSensePlugin";
        return false;
    }

    if( env->UnregisterNatives(objectClass) ) {
        qCWarning(plugin) << "Error unregistering JNI methods";
        return false;
    }*/

    qCDebug(plugin) << "JNI Native methods unregistered";
#endif // ANDROID

    // TODO: Deinit the manager
    //m_rsmanager.setup();

    RealSensePlugin::s_pInstance = nullptr;

    emit appNotice("RealSensePlugin deinitialized");
    qCDebug(plugin) << "deinit() done";
    setInitialized(false);

    return true;
}

bool RealSensePlugin::configure()
{
    qCDebug(plugin) << __func__;
    return true;
}

QVariantMap RealSensePlugin::getAvailableStreams() const
{
    qCDebug(plugin) << __func__;
    QVariantMap out;

    QMap<QString, QVariantMap> data = m_rsmanager->getAvailableStreams();
    for( auto it = data.begin(); it != data.end(); ++it )
        out[it.key()] = it.value();

    return out;
}

VideoSourceStreamObject* RealSensePlugin::getVideoStream(const QStringList path)
{
    return m_rsmanager->getVideoStream(path);
}

QList<QObject*> RealSensePlugin::listVideoStreams(const QStringList path)
{
    QList<QObject*> out;
    for( VideoSourceStreamObject* item : m_rsmanager->listVideoStreams(path) )
        out.append(item);
    return out;
}

QSharedPointer<PointCloudData> RealSensePlugin::getStreamPCData(const QString device_id)
{
    qCDebug(plugin) << __func__;
    return m_rsmanager->getStreamPointCloud(device_id);
}

void RealSensePlugin::capturePointCloudShot(const QString device_id)
{
    qCDebug(plugin) << __func__;
    m_rsmanager->capturePointCloudShot(device_id);
}
