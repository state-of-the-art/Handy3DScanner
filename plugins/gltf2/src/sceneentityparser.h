#ifndef SCENEENTITYPARSER_H
#define SCENEENTITYPARSER_H

#include <QJsonObject>
#include <QVariant>

namespace Qt3DCore {
    class QEntity;
    class QNode;
    class QTransform;
}

namespace Qt3DRender {
    class QGeometry;
    class QGeometryRenderer;
    class QMaterial;
    class QCameraLens;
    class QAttribute;
}

namespace draco {
    class EncoderBuffer;
}

namespace GLTF2 {

class SceneEntityParser
{
public:
    explicit SceneEntityParser(const Qt3DCore::QEntity *scene, const QVariantMap options);

    void process();
    QJsonObject getJson();
    QByteArray getBuffer();
    QByteArray getGlb();

protected:
    struct VertexAttrib {
        Qt3DRender::QAttribute *att;
        const char *ptr;
        quint8 size;
        QString usage;
        quint32 offset;
        quint32 stride;
        quint32 index;
    };

    void processScene();
    QJsonArray getRootTransform();

    qint32 parseEntity(const Qt3DCore::QEntity *entity);
    QJsonArray parseTransform(const Qt3DCore::QTransform *transform);
    qint32 parseMesh(const Qt3DRender::QGeometryRenderer* mesh, const Qt3DCore::QEntity *entity);
    qint32 parseMaterial(const Qt3DRender::QMaterial* material);
    qint32 parseCamera(const Qt3DRender::QCameraLens* camera);

    struct CompressedMesh {
        draco::EncoderBuffer *buffer;
        QMap<QString, int> attributes;
    };

    CompressedMesh compressMesh(const Qt3DRender::QGeometry *geometry, const QVariantMap &options);

    const Qt3DCore::QEntity *m_scene;
    QVariantMap m_options;

    QJsonObject m_json;
    QByteArray m_buffer;
};

} // namespace GLTF2

#endif // SCENEENTITYPARSER_H
