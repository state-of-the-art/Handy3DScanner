#ifndef ANDROIDWRAPPER_H
#define ANDROIDWRAPPER_H

#include <QObject>
#include <QAndroidActivityResultReceiver>

class AndroidWrapper
    : public QObject, public QAndroidActivityResultReceiver
{
    Q_OBJECT
public:
    inline static AndroidWrapper* I() { if( s_pInstance == nullptr ) s_pInstance = new AndroidWrapper(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data);
    Q_INVOKABLE int is_content_provider_supported();
    Q_INVOKABLE void choose(QString type, QString file_name = "");
    Q_INVOKABLE QString getFilePath(QString basepath, QString filename);
    Q_INVOKABLE int isAndroidProviderFile(QString path);

    static int getFileDescriptor(QString path, bool read_only = true);
    static int getFileTreeDescriptor(QString basepath, QString name, bool read_only = true);

signals:
    void filePicked(QString fileUrl);
    void filesPicked(QStringList fileUrls);
    void dirPicked(QString dirUrl);

private:
    const static int SDCARD_DOCUMENT_REQUEST = 2;
    const static int API__STORAGE_ACCESS_FRAMEWORK = 21;

    explicit AndroidWrapper(QObject *parent = nullptr);

    static AndroidWrapper *s_pInstance;

    int pick_result;
};

#endif // ANDROIDWRAPPER_H
