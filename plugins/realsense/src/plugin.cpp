#include "plugin.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(plugin, "RealSensePlugin")

#ifdef ANDROID
#    include <jni.h>
#    include <QtAndroidExtras>
#    include <QAndroidJniObject>

// Two functions from librelasense used to add/remove device
extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_DeviceWatcher_nAddUsbDevice(JNIEnv *env, jclass type, jstring deviceName_, jint fileDescriptor);
extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_DeviceWatcher_nRemoveUsbDevice(JNIEnv *env, jclass type, jint fileDescriptor);

#endif // ANDROID

QLatin1String RealSensePlugin::name() const
{
    return QLatin1String(plugin().categoryName());
}

QStringList RealSensePlugin::requirements() const
{
    qCDebug(plugin) << "requirements()";
    return QStringList();
}

bool RealSensePlugin::init()
{
    if( VideoSourceInterface::isInitialized() )
        return true;

    qCDebug(plugin) << "init()";

#ifdef ANDROID
    JavaVM* vm = QAndroidJniEnvironment::javaVM();
    JNIEnv* env;
    if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ) {
        return false;
    }

    // Attach librs native functions to the RealSensePlugin class
    JNINativeMethod methods[] {{"notifyDeviceAttached", "(Ljava/lang/String;I)V", reinterpret_cast<void *>(Java_com_intel_realsense_librealsense_DeviceWatcher_nAddUsbDevice)},
                               {"notifyDeviceDetached", "(I)V", reinterpret_cast<void *>(Java_com_intel_realsense_librealsense_DeviceWatcher_nRemoveUsbDevice)}};

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

    VideoSourceInterface::setInitialized(true);

    return true;
}

bool RealSensePlugin::deinit()
{
    if( !VideoSourceInterface::isInitialized() )
        return true;
    qCDebug(plugin) << "deinit()";

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
    jclass objectClass = env->FindClass("io/stateoftheart/handy3dscanner/plugins/RealSensePlugin");
    if( !objectClass ) {
        qCWarning(plugin) << "Couldn't find class:" << "io/stateoftheart/handy3dscanner/plugins/RealSensePlugin";
        return false;
    }

    if( env->UnregisterNatives(objectClass) ) {
        qCWarning(plugin) << "Error unregistering JNI methods";
        return false;
    }

    qCDebug(plugin) << "JNI Native methods unregistered";
#endif // ANDROID

    qCDebug(plugin) << "deinit() done";
    VideoSourceInterface::setInitialized(false);
    return true;
}

bool RealSensePlugin::configure()
{
    qCDebug(plugin) << "configure()";
    return true;
}

QStringList RealSensePlugin::getAvailableStreams() const
{
    qCDebug(plugin) << "getAvailableStreams()";
    return QStringList();
}

uint8_t *RealSensePlugin::getPCData() const
{
    qCDebug(plugin) << "getAvailableStreams()";
    return nullptr;
}
