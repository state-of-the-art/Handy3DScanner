#include "scenefile.h"

#include <Qt3DCore/QEntity>
#include <QTemporaryDir>
#include <QStandardPaths>

#include "gltf2/exporter.h"
#include "application.h"
#include "settings.h"
#ifdef ANDROID
#include "androidwrapper.h"
#endif

SceneFile* SceneFile::s_pInstance = nullptr;

SceneFile::SceneFile()
    : QObject()
{
    qDebug("Init SceneFile object");
}

SceneFile::~SceneFile()
{
    qDebug("Delete SceneFile object");
}

bool SceneFile::saveScene(Qt3DCore::QEntity *scene, const QString &file_path)
{
    qDebug() << "Saving scene to filepath" << file_path;

    QByteArray data;
    try {
        GLTF2::Exporter exporter;
        exporter.setScene(scene);
        exporter.setExportOption(QStringLiteral("mesh_compression"), Settings::I()->val("Save.glTF2.compress_file").toBool());
        exporter.setExportOption(QStringLiteral("draco.encoding_speed"), Settings::I()->val("Save.glTF2.compress_speed").toInt());
        exporter.setExportOption(QStringLiteral("draco.decoding_speed"), Settings::I()->val("Save.glTF2.compress_speed").toInt());
        exporter.setExportOption(QStringLiteral("draco.quantization.position"), Settings::I()->val("Save.glTF2.compress_quality").toInt());
        exporter.setExportOption(QStringLiteral("draco.quantization.normal"), Settings::I()->val("Save.glTF2.compress_quality").toInt());
        exporter.setExportOption(QStringLiteral("draco.quantization.color"), Settings::I()->val("Save.glTF2.compress_quality").toInt());
        exporter.setExportOption(QStringLiteral("draco.quantization.tex_coord"), Settings::I()->val("Save.glTF2.compress_quality").toInt());
        exporter.setExportOption(QStringLiteral("draco.quantization.generic"), Settings::I()->val("Save.glTF2.compress_quality").toInt());

        data = exporter.processScene();
    } catch( ... ) {
        Application::I()->error(QStringLiteral("Exception happened during export the scene: %1").arg(file_path));
        return false;
    }

    if( data.size() == 0 ) {
        Application::I()->error(QStringLiteral("The scene exporter failed to prepare data to save: %1").arg(file_path));
        return false;
    }

    // Opening file to write after preparation of the data in order to deal with limited memory
    // otherwise it is going to create file with no information at all.
#ifdef ANDROID
    QFile file;
    int fd = AndroidWrapper::getFileDescriptor(file_path, false);
    if( fd < 0 ) {
        Application::I()->error(QStringLiteral("Unable to export scene due to file descriptor is invalid: %1").arg(file_path));
        return false;
    }
    file.open(fd, QFile::WriteOnly);
#else
    QFile file(file_path);
    file.open(QIODevice::WriteOnly);
    if( !file.isWritable() ) {
        Application::I()->error(QStringLiteral("Unable to export scene due to file is not writable: %1").arg(file_path));
        return false;
    }
#endif

    qDebug() << "Write gltf2 data size:" << data.size();
    file.write(data);

    file.flush();
    file.close();

    qDebug() << "Completed write scene file" << file_path;

    return true;
}
