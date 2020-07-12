#include <jni.h>
#include <QtAndroidExtras>
#include <QAndroidJniObject>

// TODO: move to realsense provider (like in arcore)
// TODO: Restore functional
/*
// Two functions from librelasense used to add/remove device
extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_DeviceWatcher_nAddUsbDevice(JNIEnv *env, jclass type, jstring deviceName_, jint fileDescriptor);
extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_DeviceWatcher_nRemoveUsbDevice(JNIEnv *env, jclass type, jint fileDescriptor);

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    Q_UNUSED(reserved)

    JNIEnv* env;
    if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ) {
        return JNI_ERR;
    }

    // Init librealsense
    JNINativeMethod methods[] {{"notifyDeviceAttached", "(Ljava/lang/String;I)V", reinterpret_cast<void *>(Java_com_intel_realsense_librealsense_DeviceWatcher_nAddUsbDevice)},
                               {"notifyDeviceDetached", "(I)V", reinterpret_cast<void *>(Java_com_intel_realsense_librealsense_DeviceWatcher_nRemoveUsbDevice)}};

    jclass objectClass = env->FindClass("io/stateoftheart/handy3dscanner/Handy3DScannerActivity");
    if( !objectClass ) {
        qWarning() << "Couldn't find class:" << "io/stateoftheart/handy3dscanner/Handy3DScannerActivity";
        return JNI_ERR;
    }

    if( env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0])) < 0 ) {
        qWarning() << "Error registering methods: ";
        return JNI_ERR;
    } else {
        qDebug() << "JNI Native methods registered";
    }

    env->DeleteLocalRef(objectClass);

    return JNI_VERSION_1_6;
}*/
