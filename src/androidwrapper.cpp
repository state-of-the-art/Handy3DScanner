#include "androidwrapper.h"

#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QFile>
#include <QUrl>
#include "androidwrapperexception.h"
#include <QDebug>

#include "application.h"

AndroidWrapper* AndroidWrapper::s_pInstance = nullptr;

AndroidWrapper::AndroidWrapper(QObject *parent)
    : QObject(parent)
{
    qDebug() << "Init android wrapper...";
}

int AndroidWrapper::is_content_provider_supported() {
    return (QtAndroid::androidSdkVersion() >= API__STORAGE_ACCESS_FRAMEWORK);
}

void AndroidWrapper::handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data) {
    int RESULT_OK = QAndroidJniObject::getStaticField<int>("android/app/Activity", "RESULT_OK");
    if( receiverRequestCode == SDCARD_DOCUMENT_REQUEST && resultCode == RESULT_OK && data.isValid() ) {
        QStringList picked;
        QAndroidJniObject uri = data.callObjectMethod("getClipData","()Landroid/content/ClipData;");
        if( uri.isValid() ) {
            int clipdata_len = uri.callMethod<jint>("getItemCount");
            for( int i = 0; i < clipdata_len; i++ ) {
                QAndroidJniObject item = uri.callObjectMethod("getItemAt", "(I)Landroid/content/ClipData$Item;", i);
                QAndroidJniObject data = item.callObjectMethod("getUri", "()Landroid/net/Uri;");
                picked.append(data.toString());
            }
        } else {
            uri = data.callObjectMethod("getData", "()Landroid/net/Uri;");
            if( ! uri.isValid() ) {
                Application::I()->error(QLatin1String("Unable to read uri/s from the android content provider"));
                return;
            }
            picked.append(uri.toString());
        }

        jint FLAG_GRANT_READ_URI_PERMISSION = QAndroidJniObject::getStaticField<jint>(
                    "android.content.Intent", "FLAG_GRANT_READ_URI_PERMISSION");
        jint FLAG_GRANT_WRITE_URI_PERMISSION = QAndroidJniObject::getStaticField<jint>(
                    "android.content.Intent", "FLAG_GRANT_WRITE_URI_PERMISSION");

        int takeFlags = data.callMethod<jint>("getFlags", "()I");
        takeFlags &= ( FLAG_GRANT_READ_URI_PERMISSION | FLAG_GRANT_WRITE_URI_PERMISSION );
        QAndroidJniObject contentResolver = QtAndroid::androidActivity()
                .callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");

        for( QString path : picked ) {
            contentResolver.callMethod<void>("takePersistableUriPermission","(Landroid/net/Uri;I)V",
                                             uri.object<jobject>(), takeFlags);
        }

        if( pick_result == 0 )
            emit filesPicked(picked);
        else if( pick_result == 1 )
            emit filePicked(picked.first());
        else if( pick_result == 2 )
            emit dirPicked(picked.first());
    }
}

void AndroidWrapper::choose(QString type, QString file_name) {
    QAndroidJniObject intent("android/content/Intent");
    QAndroidJniObject setAction;
    QAndroidJniObject setType;
    QAndroidJniObject setFileName;

    pick_result = -1;
    if( type == "openFile" ) {
        setAction = QAndroidJniObject::fromString("android.intent.action.OPEN_DOCUMENT");
        setType = QAndroidJniObject::fromString("*/*");
        pick_result = 0;
    } else if( type == "saveFile" ) {
        setAction = QAndroidJniObject::fromString("android.intent.action.CREATE_DOCUMENT");
        setType = QAndroidJniObject::fromString("*/*");
        setFileName = QAndroidJniObject::fromString(file_name);
        pick_result = 1;
    } else if( type == "selectDirectory" ) {
        setAction = QAndroidJniObject::fromString("android.intent.action.OPEN_DOCUMENT_TREE");
        pick_result = 2;
    }

    QAndroidJniObject extra_title = QAndroidJniObject::fromString("android.intent.extra.TITLE");
    QAndroidJniObject extra_multiple = QAndroidJniObject::fromString("android.intent.extra.ALLOW_MULTIPLE");
    QAndroidJniObject extra_advanced = QAndroidJniObject::fromString("android.content.extra.SHOW_ADVANCED");
    //QAndroidJniObject category = QAndroidJniObject::getStaticObjectField("android/content/Intent", "CATEGORY_OPENABLE", "Ljava/lang/String;");

    if( setAction.isValid() && intent.isValid() ) {
        intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", setAction.object<jstring>());
        if( setType.isValid() )
            intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", setType.object<jstring>());

        //intent.callObjectMethod("addCategory", "(Ljava/lang/String;)Landroid/content/Intent;", category.object<jstring>());

        if( setFileName.isValid() )
            intent.callObjectMethod("putExtra","(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", extra_title.object<jstring>(), setFileName.object<jstring>());
        if( type == "openFile" )
            intent.callObjectMethod("putExtra","(Ljava/lang/String;Z)Landroid/content/Intent;", extra_multiple.object<jstring>(), jboolean(true));

        intent.callObjectMethod("putExtra","(Ljava/lang/String;Z)Landroid/content/Intent;", extra_advanced.object<jstring>(), jboolean(true));

        QtAndroid::startActivity(intent.object<jobject>(), SDCARD_DOCUMENT_REQUEST, this);
    } else
        throw AndroidWrapperException(QString("JNI intent action is invalid: %1").arg(setAction.toString()));
}

QString AndroidWrapper::getFilePath(QString basepath, QString filename)
{
    if( isAndroidProviderFile(basepath) )
        return basepath + QUrl::toPercentEncoding("/" + filename.replace('/', '_'));
    else
        return basepath + "/" + filename.replace('/', '_');
}

int AndroidWrapper::isAndroidProviderFile(QString path) {
    return path.startsWith("content://");
}

int AndroidWrapper::getFileDescriptor(QString path, bool read_only) {
    QAndroidJniObject jpath = QAndroidJniObject::fromString(path);
    QAndroidJniObject jmode = QAndroidJniObject::fromString(read_only ? "r" : "w");
    QAndroidJniObject uri = QAndroidJniObject::callStaticObjectMethod("android/net/Uri",
            "parse", "(Ljava/lang/String;)Landroid/net/Uri;", jpath.object<jstring>());

    QAndroidJniObject contentResolver = QtAndroid::androidActivity()
            .callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");

    QAndroidJniObject parcelFileDescriptor = contentResolver
            .callObjectMethod("openFileDescriptor",
            "(Landroid/net/Uri;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
            uri.object<jobject>(), jmode.object<jobject>());

    QAndroidJniEnvironment env;
    if( env->ExceptionCheck() ) {
        qWarning() << "Found exception during getting file descriptor:";
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return parcelFileDescriptor.callMethod<jint>("getFd", "()I");
}

int AndroidWrapper::getFileTreeDescriptor(QString basepath, QString name, bool read_only) {
    QAndroidJniObject jpath = QAndroidJniObject::fromString(basepath);
    QAndroidJniObject jmode = QAndroidJniObject::fromString(read_only ? "r" : "w");
    QAndroidJniObject uri = QAndroidJniObject::callStaticObjectMethod("android/net/Uri",
            "parse", "(Ljava/lang/String;)Landroid/net/Uri;", jpath.object<jstring>());

    QAndroidJniObject docId = QAndroidJniObject::callStaticObjectMethod("android/provider/DocumentsContract",
            "getTreeDocumentId", "(Landroid/net/Uri;)Ljava/lang/String;", uri.object<jobject>());

    QAndroidJniObject docUri = QAndroidJniObject::callStaticObjectMethod("android/provider/DocumentsContract",
            "buildDocumentUriUsingTree", "(Landroid/net/Uri;Ljava/lang/String;)Landroid/net/Uri;",
            uri.object<jobject>(), docId.object<jstring>());

    QAndroidJniObject contentResolver = QtAndroid::androidActivity()
            .callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");

    QAndroidJniObject fileUri = QAndroidJniObject::callStaticObjectMethod("android/provider/DocumentsContract",
            "createDocument", "(Landroid/content/ContentResolver;Landroid/net/Uri;Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;",
            contentResolver.object<jobject>(), docUri.object<jobject>(),
            QAndroidJniObject::fromString("application/octet-stream").object<jstring>(), QAndroidJniObject::fromString(name).object<jstring>());

    QAndroidJniObject parcelFileDescriptor = contentResolver
            .callObjectMethod("openFileDescriptor",
            "(Landroid/net/Uri;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
            fileUri.object<jobject>(), jmode.object<jobject>());

    QAndroidJniEnvironment env;
    if( env->ExceptionCheck() ) {
        qWarning() << "Found exception during getting file descriptor:";
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return parcelFileDescriptor.callMethod<jint>("getFd", "()I");
}
