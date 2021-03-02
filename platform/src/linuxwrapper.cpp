#include "linuxwrapper.h"

#include <QDebug>

#include <QUrl>

LinuxWrapper* LinuxWrapper::s_pInstance = nullptr;

LinuxWrapper::LinuxWrapper(QObject *parent)
    : QObject(parent)
{
    qDebug("Init linux wrapper...");
}

void LinuxWrapper::choose(QString type, QString file_name) {
    QString path;

    if( type == "openFile" ) {
        // DEBUG
        QStringList files;
        files.append(QStringLiteral("/home/user/Work/state-of-the-art/Handy3DScanner.wiki/examples/v0.5.0/shoe/shot_20190714_150809.pcd"));
        files.append(QStringLiteral("/home/user/Work/state-of-the-art/Handy3DScanner.wiki/examples/v0.5.0/shoe/shot_20190714_150817.pcd"));
        files.append(QStringLiteral("/home/user/Work/state-of-the-art/Handy3DScanner.wiki/examples/v0.5.0/shoe/shot_20190714_150824.pcd"));
        emit filesPicked(files);
    } else if( type == "saveFile" ) {
        // DEBUG
        emit filePicked(QStringLiteral("/tmp/%1").arg(file_name));
    } else if( type == "selectDirectory" ) {
        // DEBUG
        emit dirPicked(QStringLiteral("/tmp"));
    }
}
