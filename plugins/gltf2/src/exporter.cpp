#include "exporter.h"

#include <Qt3DCore/QEntity>
#include <QJsonObject>

#include "sceneentityparser.h"

GLTF2::Exporter::Exporter()
    : QObject()
    , m_scene(nullptr)
    , m_json(nullptr)
{
    qDebug("Creating GLTF2Exporter object");

    m_options[QStringLiteral("mesh_compression")] = true;
    m_options[QStringLiteral("draco.decoding_speed")] = 0; // Best decompression
    m_options[QStringLiteral("draco.encoding_speed")] = 0; // Best compression
    m_options[QStringLiteral("draco.quantization.position")] = 16;
    m_options[QStringLiteral("draco.quantization.normal")] = 16;
    m_options[QStringLiteral("draco.quantization.color")] = 16;
    m_options[QStringLiteral("draco.quantization.tex_coord")] = 16;
    m_options[QStringLiteral("draco.quantization.generic")] = 16;
}

GLTF2::Exporter::~Exporter()
{
    qDebug("Destroying GLTF2Exporter object");
    delete m_json;
}

void GLTF2::Exporter::setScene(Qt3DCore::QEntity *scene)
{
    m_scene = scene;
}

QByteArray GLTF2::Exporter::processScene()
{
    if( m_scene == nullptr ) {
        qWarning("Run process without the scene defined");
        return QByteArray();
    }

    GLTF2::SceneEntityParser parser(m_scene, m_options);
    parser.process();
    qDebug() << parser.getJson();
    return parser.getGlb();
}

void GLTF2::Exporter::setExportOption(const QString &key, const QVariant val)
{
    m_options[key] = val;
}

QVariant GLTF2::Exporter::getExportOption(const QString &key)
{
    return m_options[key];
}
