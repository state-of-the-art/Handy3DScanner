#ifndef LINUXWRAPPER_H
#define LINUXWRAPPER_H

#include <QObject>

class LinuxWrapper
    : public QObject
{
    Q_OBJECT
public:
    inline static LinuxWrapper* I() { if( s_pInstance == nullptr ) s_pInstance = new LinuxWrapper(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    Q_INVOKABLE void choose(QString type, QString file_name = "");

signals:
    void filePicked(QString fileUrl);
    void filesPicked(QStringList fileUrls);
    void dirPicked(QString dirUrl);

private:
    explicit LinuxWrapper(QObject *parent = nullptr);
    static LinuxWrapper *s_pInstance;
};

#endif // LINUXWRAPPER_H
