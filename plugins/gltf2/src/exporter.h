#ifndef GLTF2EXPORTER_H
#define GLTF2EXPORTER_H

#include <QObject>
#include <QVariant>

namespace Qt3DCore {
    class QEntity;
}

namespace GLTF2 {

class Exporter : public QObject
{
    Q_OBJECT
public:
    explicit Exporter();
    ~Exporter();

    void setScene(Qt3DCore::QEntity *scene);

    QByteArray processScene();

    void setExportOption(const QString &key, const QVariant val);
    QVariant getExportOption(const QString &key);

private:
    QVariantMap m_options;
    Qt3DCore::QEntity *m_scene;
    QJsonObject *m_json;
};

} // Namespace GLTF2

#endif // GLTF2EXPORTER_H
