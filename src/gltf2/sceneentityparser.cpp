#include "sceneentityparser.h"

#include <QLoggingCategory>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGeometryFactory>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QAbstractLight>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtMath>

#include <QCoreApplication>

#include <QOpenGLContext>

#include "constants.h"

#include <draco/compression/encode.h>
#include <draco/mesh/mesh.h>
#include <draco/attributes/geometry_attribute.h>
#include <draco/attributes/geometry_indices.h>
#include <draco/io/mesh_io.h>

const QString VERTICES_ATTRIBUTE_NAME = Qt3DRender::QAttribute::defaultPositionAttributeName();
const QString NORMAL_ATTRIBUTE_NAME =  Qt3DRender::QAttribute::defaultNormalAttributeName();
const QString TANGENT_ATTRIBUTE_NAME = Qt3DRender::QAttribute::defaultTangentAttributeName();
const QString TEXTCOORD_ATTRIBUTE_NAME = Qt3DRender::QAttribute::defaultTextureCoordinateAttributeName();
const QString COLOR_ATTRIBUTE_NAME = Qt3DRender::QAttribute::defaultColorAttributeName();

namespace GLTF2 {
Q_LOGGING_CATEGORY(sceneentityparser, "SceneEntityParser")
Q_LOGGING_CATEGORY(dracocompressor, "DracoCompressor")

SceneEntityParser::SceneEntityParser(const Qt3DCore::QEntity *scene, const QVariantMap options)
    : m_scene(scene)
    , m_options(options)
{
    qCDebug(sceneentityparser) << "Creating the parser";
}

void SceneEntityParser::process()
{
    processScene();

    // Apply asset config
    QJsonObject asset;
    asset[KEY_GENERATOR] = QCoreApplication::applicationName()+QStringLiteral(" ")+QCoreApplication::applicationVersion();
    asset[KEY_VERSION] = "2.0";
    m_json[KEY_ASSET] = asset;

    // Add draco extension if compression enabled
    if( m_options[QStringLiteral("mesh_compression")].toBool() ) {
        QJsonArray ext_used = m_json[KEY_EXTENSIONS_USED].toArray();
        QJsonArray ext_required = m_json[KEY_EXTENSIONS_USED].toArray();

        ext_used.append(KEY_EXT_DRACO_COMP);
        ext_required.append(KEY_EXT_DRACO_COMP);

        m_json[KEY_EXTENSIONS_USED] = ext_used;
        m_json[KEY_EXTENSIONS_REQ] = ext_required;
    }
}

void SceneEntityParser::processScene()
{
    QJsonObject item;
    qint32 id = m_json[KEY_SCENES].toArray().count();

    QJsonArray nodes;
    nodes.append(parseEntity(m_scene));

    item[KEY_NODES] = nodes;

    QJsonArray items = m_json[KEY_SCENES].toArray();
    items.insert(id, item);
    m_json[KEY_SCENES] = items;

    if( !m_json.contains(KEY_SCENE) )
        m_json[KEY_SCENE] = id;
}

QJsonObject SceneEntityParser::getJson()
{
    return m_json;
}

QByteArray SceneEntityParser::getBuffer()
{
    return m_buffer;
}

QByteArray SceneEntityParser::getGlb()
{
    QByteArray out;

    if( m_buffer.isEmpty() ) {
        qCWarning(sceneentityparser, "Unable to create GLB blob without filled buffer");
        return out;
    }

    quint32 chunk_length;
    quint32 chunk_type = 0x4E4F534A; // JSON chunk type

    // Modifying buffers in json
    QJsonObject buffer;
    buffer[KEY_BYTE_LENGTH] = m_buffer.size();
    QJsonArray buffers;
    buffers.append(buffer);
    m_json[KEY_BUFFERS] = buffers;

    // Storing json in the out byte array
    out = QJsonDocument(m_json).toJson(QJsonDocument::Compact);
    chunk_length = quint32(out.size());
    if( chunk_length % 4 > 0 ) { // Align chunk to 4 byte
        out.append(4 - chunk_length % 4, ' ');
        chunk_length = quint32(out.size());
    }
    out.prepend(reinterpret_cast<const char *>(&chunk_type), sizeof(quint32));
    out.prepend(reinterpret_cast<const char *>(&chunk_length), sizeof(quint32));

    // Processing the data buffer
    chunk_type = quint32(0x004E4942); // BIN chunk type
    chunk_length = quint32(m_buffer.size());
    out.append(reinterpret_cast<const char *>(&chunk_length), sizeof(quint32));
    out.append(reinterpret_cast<const char *>(&chunk_type), sizeof(quint32));
    out.append(m_buffer);

    // Prepending with header
    quint32 gltfv_version(2);
    quint32 gltf_length = quint32(out.size())+sizeof(quint32)*3; // + header size 12b
    out.prepend(reinterpret_cast<const char *>(&gltf_length), sizeof(quint32));
    out.prepend(reinterpret_cast<const char *>(&gltfv_version), sizeof(quint32));
    out.prepend("glTF");

    return out;
}

qint32 SceneEntityParser::parseEntity(const Qt3DCore::QEntity *entity)
{
    if( !entity || !entity->isEnabled() )
        return -1;

    QJsonObject item;
    qint32 id = m_json[KEY_NODES].toArray().count();
    if( !entity->objectName().isEmpty() )
        item[KEY_NAME] = entity->objectName();

    const auto components = entity->components();
    for( auto component : components ) {
        if( auto mesh = qobject_cast<Qt3DRender::QGeometryRenderer*>(component) )
            item[KEY_MESH] = parseMesh(mesh, entity);
        else if( auto transform = qobject_cast<Qt3DCore::QTransform*>(component) )
            item[KEY_MATRIX] = parseTransform(transform);
        else if( auto camera = qobject_cast<Qt3DRender::QCameraLens*>(component) )
            item[KEY_CAMERA] = parseCamera(camera);
        else
            qCDebug(sceneentityparser) << "Skipping entity component" << entity;
    }

    if( id == 0 )
        item[KEY_MATRIX] = getRootTransform();

    QJsonArray items = m_json[KEY_NODES].toArray();
    items.insert(id, item);
    m_json[KEY_NODES] = items;

    QJsonArray childrens;

    for( auto child : entity->children() ) {
        qint32 children_id = parseEntity(qobject_cast<Qt3DCore::QEntity*>(child));
        if( children_id > -1 )
            childrens.append(children_id);
    }

    if( childrens.size() > 0 )
        item[KEY_CHILDREN] = childrens;

    items = m_json[KEY_NODES].toArray();
    items.replace(id, item);
    m_json[KEY_NODES] = items;

    return id;
}

QJsonArray SceneEntityParser::getRootTransform()
{
    Qt3DCore::QTransform root_transform;
    QMatrix4x4 matrix;

    // Rotating transformation of root node to display scene properly in 3d viewers/editors
    matrix.rotate(180.0f, QVector3D(0.0f, 1.0f, 0.0f)); // Making sure front will be by Y
    matrix.rotate(180.0f, QVector3D(0.0f, 0.0f, 1.0f)); // Inverting Z axis to make use down is down
    root_transform.setMatrix(matrix);

    return parseTransform(&root_transform);
}

QJsonArray SceneEntityParser::parseTransform(const Qt3DCore::QTransform *transform)
{
    QJsonArray matrix;
    for( int j = 0; j < 4; ++j ) {
        // Access the constData of the matrix is not working on arm64 android
        // and producing a weird outputs: https://github.com/state-of-the-art/Handy3DScanner/issues/9#issuecomment-503406420
        QVector4D col(transform->matrix().column(j));
        matrix.append(col.x());
        matrix.append(col.y());
        matrix.append(col.z());
        matrix.append(col.w());
    }
    return matrix;
}

qint32 SceneEntityParser::parseMesh(const Qt3DRender::QGeometryRenderer *mesh, const Qt3DCore::QEntity *entity)
{
    QJsonObject json_mesh;
    qint32 id = m_json[KEY_MESHES].toArray().count();
    if( !mesh->objectName().isEmpty() )
        json_mesh[KEY_NAME] = mesh->objectName();

    Qt3DRender::QGeometryFactoryPtr geometry_factory = mesh->geometryFactory();
    Qt3DRender::QGeometry *geometry = geometry_factory.data() ? geometry_factory.data()->operator()() : mesh->geometry();

    if( !geometry ) {
        qCWarning(sceneentityparser) << "Ignoring mesh without geometry";
        return -1;
    }

    // Creating vector of attributes
    QVector<VertexAttrib> vector_attr;
    vector_attr.reserve(geometry->attributes().size());

    Qt3DRender::QAttribute *index_attr(nullptr);
    const quint16 *index_ptr(nullptr);
    quint32 index_count(0);
    quint32 index_size(0);

    qint32 vertex_buffer_stride(0);

    const auto attributes = geometry->attributes();
    for( Qt3DRender::QAttribute *att : attributes ) {
        if( att->attributeType() == Qt3DRender::QAttribute::IndexAttribute ) {
            index_attr = att;
            index_ptr = reinterpret_cast<const quint16*>(att->buffer()->data().constData());
            index_count = index_attr->count();
            index_size = index_attr->vertexBaseType() == Qt3DRender::QAttribute::UnsignedShort ? sizeof(quint16) : sizeof(quint32);
        } else {
            VertexAttrib v_attr;
            if( att->name() == VERTICES_ATTRIBUTE_NAME )
                v_attr.usage = QStringLiteral("POSITION");
            else if( att->name() == NORMAL_ATTRIBUTE_NAME )
                v_attr.usage = QStringLiteral("NORMAL");
            else if( att->name() == TEXTCOORD_ATTRIBUTE_NAME )
                v_attr.usage = QStringLiteral("TEXCOORD_0");
            else if( att->name() == COLOR_ATTRIBUTE_NAME )
                v_attr.usage = QStringLiteral("COLOR_0");
            else if( att->name() == TANGENT_ATTRIBUTE_NAME )
                v_attr.usage = QStringLiteral("TANGENT");
            else {
                //v_attr.usage = att->name();
                qCDebug(sceneentityparser) << "Ignoring non-standard attribute" << att->name();
                continue;
            }
            v_attr.att = att;
            v_attr.ptr = att->buffer()->data().constData();

            v_attr.offset = att->byteOffset() / v_attr.size;
            v_attr.index = v_attr.offset;
            v_attr.stride = att->byteStride();
            v_attr.size = v_attr.stride / att->vertexSize();
            vertex_buffer_stride += v_attr.stride;

            vector_attr << v_attr;
        }
    }

    qint32 attr_count(vector_attr.size());
    if( !attr_count ) {
        qCWarning(sceneentityparser, "Ignoring mesh without attributes");
        return -1;
    }

    const quint32 vertex_count = vector_attr.at(0).att->count();
    qint32 vertex_buffer_view_id(-1);
    qint32 index_view_id(-1);

    QJsonArray buffer_views;
    QJsonObject primitive;

    if( m_options[QStringLiteral("mesh_compression")].toBool() ) {
        auto compressed = compressMesh(geometry, m_options);
        if( !compressed.buffer ) {
            qCWarning(sceneentityparser, "Invalid compressed buffer");
            return -1;
        }

        // Creating view for the compressed buffer
        QJsonObject comp_buffer_view;
        qint32 comp_buffer_view_id = m_json[KEY_BUFFER_VIEWS].toArray().count();
        comp_buffer_view[KEY_BUFFER] = 0;
        comp_buffer_view[KEY_BYTE_OFFSET] = m_buffer.size();
        comp_buffer_view[KEY_BYTE_LENGTH] = qint32(compressed.buffer->size());

        buffer_views = m_json[KEY_BUFFER_VIEWS].toArray();
        buffer_views.insert(comp_buffer_view_id, comp_buffer_view);
        m_json[KEY_BUFFER_VIEWS] = buffer_views;

        // Attaching buffer to the buffer binary
        m_buffer.append(compressed.buffer->data(), qint32(compressed.buffer->size()));
        delete compressed.buffer;

        QJsonObject draco_extension;
        draco_extension[KEY_BUFFER_VIEW] = comp_buffer_view_id;
        QJsonObject draco_attributes;
        for( const auto &attr : compressed.attributes.keys() )
            draco_attributes[attr] = compressed.attributes[attr];
        draco_extension[KEY_ATTRIBUTES] = draco_attributes;

        QJsonObject extensions = primitive[KEY_EXTENSIONS].toObject();
        extensions[KEY_EXT_DRACO_COMP] = draco_extension;
        primitive[KEY_EXTENSIONS] = extensions;
    } else {
        // Converting vertex attributes data into buffer
        QByteArray vertex_buf;
        vertex_buf.resize(vertex_buffer_stride*vertex_count);

        char *p = reinterpret_cast<char*>(vertex_buf.data());

        for( quint32 i = 0; i < vertex_count; ++i ) {
            for( qint32 j = 0; j < attr_count; ++j ) {
                VertexAttrib &vertex_attr = vector_attr[j];
                for( quint32 k = 0; k < vertex_attr.stride; ++k )
                    *p++ = vertex_attr.ptr[vertex_attr.index++];
            }
        }

        QJsonObject vertex_buffer_view;
        vertex_buffer_view_id = m_json[KEY_BUFFER_VIEWS].toArray().count();
        vertex_buffer_view[KEY_BUFFER] = 0;
        vertex_buffer_view[KEY_BYTE_OFFSET] = m_buffer.size();
        vertex_buffer_view[KEY_BYTE_LENGTH] = vertex_buf.size();
        vertex_buffer_view[KEY_BYTE_STRIDE] = vertex_buffer_stride;
        vertex_buffer_view[KEY_TARGET] = qint32(GL_ARRAY_BUFFER);

        buffer_views = m_json[KEY_BUFFER_VIEWS].toArray();
        buffer_views.insert(vertex_buffer_view_id, vertex_buffer_view);
        m_json[KEY_BUFFER_VIEWS] = buffer_views;

        m_buffer.append(vertex_buf);

        // Creating index buffer
        QByteArray index_buf;
        index_view_id = m_json[KEY_BUFFER_VIEWS].toArray().count();
        QJsonObject index_buffer_view;
        if( index_attr ) {
            uint src_index = index_attr->byteOffset() / index_size;
            const quint32 index_stride = index_attr->byteStride() ? index_attr->byteStride() / index_size - 1 : 0;
            index_buf.resize(index_count * index_size);
            if( index_size == sizeof(quint32) ) {
                quint32 *dst = reinterpret_cast<quint32*>(index_buf.data());
                const quint32 *src = reinterpret_cast<const quint32*>(index_ptr);
                for( quint32 j = 0; j < index_count; ++j ) {
                    *dst++ = src[src_index++];
                    src_index += index_stride;
                }
            } else {
                quint16 *dst = reinterpret_cast<quint16 *>(index_buf.data());
                for( quint32 j = 0; j < index_count; ++j ) {
                    *dst++ = index_ptr[src_index++];
                    src_index += index_stride;
                }
            }

            index_buffer_view[KEY_BUFFER] = 0;
            index_buffer_view[KEY_BYTE_LENGTH] = index_buf.size();
            index_buffer_view[KEY_BYTE_OFFSET] = m_buffer.size();
            index_buffer_view[KEY_TARGET] = qint32(GL_ELEMENT_ARRAY_BUFFER);

            buffer_views = m_json[KEY_BUFFER_VIEWS].toArray();
            buffer_views.insert(index_view_id, index_buffer_view);
            m_json[KEY_BUFFER_VIEWS] = buffer_views;
            m_buffer.append(index_buf);
        }
    }

    // Accessors
    qint32 start_offset(0);

    QJsonObject primitive_attrs;

    QJsonArray accessors = m_json[KEY_ACCESSORS].toArray();
    QJsonObject accessor;
    if( !m_options[QStringLiteral("mesh_compression")].toBool() )
        accessor[KEY_BUFFER_VIEW] = vertex_buffer_view_id;
    accessor[KEY_COUNT] = qint32(vertex_count);
    for( int i = 0; i < attr_count; ++i ) {
        const VertexAttrib &attr = vector_attr.at(i);

        primitive_attrs[attr.usage] = accessors.size();

        if( !m_options[QStringLiteral("mesh_compression")].toBool() )
            accessor[KEY_BYTE_OFFSET] = start_offset;
        switch( attr.att->vertexSize() ) {
        case 1:
            accessor[KEY_TYPE] = QStringLiteral("SCALAR");
            break;
        case 2:
            accessor[KEY_TYPE] = QStringLiteral("VEC2");
            break;
        case 3:
            accessor[KEY_TYPE] = QStringLiteral("VEC3");
            break;
        case 4:
            accessor[KEY_TYPE] = QStringLiteral("VEC4");
            break;
        case 9:
            accessor[KEY_TYPE] = QStringLiteral("MAT3");
            break;
        case 16:
            accessor[KEY_TYPE] = QStringLiteral("MAT4");
            break;
        default:
            qCWarning(sceneentityparser, "Invalid vertex size: %d", attr.att->vertexSize());
            break;
        }

        switch( attr.att->vertexBaseType() ) {
        case Qt3DRender::QAttribute::Byte :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_BYTE);
            accessor["normalized"] = true;
            break;
        case Qt3DRender::QAttribute::UnsignedByte :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_UNSIGNED_BYTE);
            accessor["normalized"] = true;
            break;
        case Qt3DRender::QAttribute::Short :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_SHORT);
            accessor["normalized"] = true;
            break;
        case Qt3DRender::QAttribute::UnsignedShort :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_UNSIGNED_SHORT);
            accessor["normalized"] = true;
            break;
        case Qt3DRender::QAttribute::Int :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_INT);
            break;
        case Qt3DRender::QAttribute::UnsignedInt :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_UNSIGNED_INT);
            break;
        case Qt3DRender::QAttribute::Float :
            accessor[KEY_COMPONENT_TYPE] = qint32(GL_FLOAT);
            break;
        }
        accessors.append(accessor);
        accessor.remove("normalized");
        start_offset += attr.stride;
    }

    primitive[KEY_ATTRIBUTES] = primitive_attrs;
    primitive[KEY_MODE] = 0; // POINTS

    if( index_attr ) {
        primitive[KEY_INDICES] = accessors.size();
        primitive[KEY_MODE] = 4; // TRIANGLES
        if( !m_options[QStringLiteral("mesh_compression")].toBool() ) {
            accessor[KEY_BUFFER_VIEW] = index_view_id;
            accessor[KEY_BYTE_OFFSET] = 0;
        }
        accessor[KEY_COUNT] = qint32(index_count);
        accessor[KEY_COMPONENT_TYPE] = qint32(index_size == sizeof(quint32) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT);;
        accessor[KEY_TYPE] = QStringLiteral("SCALAR");
        accessors.append(accessor);
    }

    m_json[KEY_ACCESSORS] = accessors;

    // Search for primitive material
    const auto components = entity->components();
    for( auto component : components ) {
        if( auto material = qobject_cast<Qt3DRender::QMaterial*>(component) )
            primitive[KEY_MATERIAL] = parseMaterial(material);
    }

    QJsonArray primitives;
    primitives.append(primitive);
    json_mesh[KEY_PRIMITIVES] = primitives;

    QJsonArray meshes;
    meshes = m_json[KEY_MESHES].toArray();
    meshes.insert(id, json_mesh);
    m_json[KEY_MESHES] = meshes;

    return id;

}

qint32 SceneEntityParser::parseMaterial(const Qt3DRender::QMaterial *material)
{
    QJsonObject item;
    qint32 id = m_json[KEY_MATERIALS].toArray().count();
    if( !material->objectName().isEmpty() )
        item[KEY_NAME] = material->objectName();

    // TODO

    QJsonArray items = m_json[KEY_MATERIALS].toArray();
    items.insert(id, item);
    m_json[KEY_MATERIALS] = items;

    return id;
}

qint32 SceneEntityParser::parseCamera(const Qt3DRender::QCameraLens *camera)
{
    QJsonObject item;
    qint32 id = m_json[KEY_CAMERAS].toArray().count();
    if( !camera->objectName().isEmpty() )
        item[KEY_NAME] = camera->objectName();

    QJsonObject params;

    params[KEY_ZNEAR] = static_cast<double>(camera->nearPlane());
    params[KEY_ZFAR] = static_cast<double>(camera->farPlane());

    if( camera->projectionType() == Qt3DRender::QCameraLens::PerspectiveProjection ) {
        item[KEY_TYPE] = KEY_PERSPECTIVE;
        params[KEY_ASPECT_RATIO] = static_cast<double>(camera->aspectRatio());
        params[KEY_YFOV] = static_cast<double>(qDegreesToRadians(camera->fieldOfView()));
    } else {
        item[KEY_TYPE] = KEY_ORTHOGRAPHIC;
        params[KEY_XMAG] = static_cast<double>(qAbs(camera->left() - camera->right()));
        params[KEY_YMAG] = static_cast<double>(qAbs(camera->top() - camera->bottom()));
    }

    item[item[KEY_TYPE].toString()] = params;

    QJsonArray items = m_json[KEY_CAMERAS].toArray();
    items.insert(id, item);
    m_json[KEY_CAMERAS] = items;

    return id;
}





namespace {
draco::GeometryAttribute::Type attributeTypeFromName(const QString &attribute_name)
{
    auto type = draco::GeometryAttribute::Type::INVALID;

    if( attribute_name == VERTICES_ATTRIBUTE_NAME )
        type = draco::GeometryAttribute::Type::POSITION;
    else if( attribute_name == NORMAL_ATTRIBUTE_NAME )
        type = draco::GeometryAttribute::Type::NORMAL;
    else if( attribute_name == COLOR_ATTRIBUTE_NAME )
        type = draco::GeometryAttribute::Type::COLOR;
    else if( attribute_name == TEXTCOORD_ATTRIBUTE_NAME ||
             attribute_name == Qt3DRender::QAttribute::defaultTextureCoordinate1AttributeName() ||
             attribute_name == Qt3DRender::QAttribute::defaultTextureCoordinate2AttributeName() )
        type = draco::GeometryAttribute::Type::TEX_COORD;
    else if( attribute_name == TANGENT_ATTRIBUTE_NAME ||
             attribute_name == Qt3DRender::QAttribute::defaultJointWeightsAttributeName() ||
             attribute_name == Qt3DRender::QAttribute::defaultJointIndicesAttributeName() )
        type = draco::GeometryAttribute::Type::GENERIC;

    return type;
}

template<typename T>
struct toDracoDataType {
    static constexpr draco::DataType type = draco::DataType::DT_INVALID;
};

template<>
struct toDracoDataType<float> {
    static constexpr draco::DataType type = draco::DataType::DT_FLOAT32;
};

template<>
struct toDracoDataType<qint8> {
    static constexpr draco::DataType type = draco::DataType::DT_INT8;
};

template<>
struct toDracoDataType<quint8> {
    static constexpr draco::DataType type = draco::DataType::DT_UINT8;
};

template<>
struct toDracoDataType<quint16> {
    static constexpr draco::DataType type = draco::DataType::DT_UINT16;
};

template<>
struct toDracoDataType<qint16> {
    static constexpr draco::DataType type = draco::DataType::DT_INT16;
};

template<>
struct toDracoDataType<qint32> {
    static constexpr draco::DataType type = draco::DataType::DT_INT32;
};

template<>
struct toDracoDataType<quint32> {
    static constexpr draco::DataType type = draco::DataType::DT_UINT32;
};

template<>
struct toDracoDataType<double> {
    static constexpr draco::DataType type = draco::DataType::DT_FLOAT64;
};

template<typename T, typename Sz>
static constexpr Sz actualStride(Sz gl_stride, Sz vertex_size) {
    return gl_stride == 0 ? vertex_size * Sz(sizeof(T)) : gl_stride;
}

template<typename T>
QByteArray packedDataInBuffer(const QByteArray &input_buffer,
                              std::size_t vertex_size,
                              std::size_t offset,
                              std::size_t stride,
                              std::size_t count)
{
    const quint64 element_size = vertex_size * sizeof(T);

    Q_ASSERT_X(count < std::numeric_limits<qint32>::max() / element_size, "GLTS2::packedDataInBuffer", "Too many elements for a QByteArray");

    QByteArray outputBuffer(int(count * element_size), Qt::Uninitialized);

    auto byte_marker_position = offset;
    stride = actualStride<T>(stride, vertex_size);

    for( std::size_t i = 0; i < count; ++i ) {
        std::memcpy(&(outputBuffer.data()[i * element_size]), &(input_buffer.data()[byte_marker_position]), element_size);
        byte_marker_position += stride;
    }

    return outputBuffer;
}

QString semanticNameFromAttribute(const Qt3DRender::QAttribute &attribute)
{
    QString str = attribute.property("semanticName").toString();
    if( !str.isEmpty() ) {
        return str;
    } else {
        const QString name = attribute.name();
        if( name == VERTICES_ATTRIBUTE_NAME )
            return QStringLiteral("POSITION");
        if( name == NORMAL_ATTRIBUTE_NAME )
            return QStringLiteral("NORMAL");
        if( name == TANGENT_ATTRIBUTE_NAME )
            return QStringLiteral("TANGENT");
        if( name == TEXTCOORD_ATTRIBUTE_NAME )
            return QStringLiteral("TEXCOORD_0");
        if( name == Qt3DRender::QAttribute::defaultTextureCoordinate1AttributeName() )
            return QStringLiteral("TEXCOORD_1");
        if( name == Qt3DRender::QAttribute::defaultTextureCoordinate2AttributeName() )
            return QStringLiteral("TEXCOORD_2");
        if( name == COLOR_ATTRIBUTE_NAME )
            return QStringLiteral("COLOR_0");
        if( name == Qt3DRender::QAttribute::defaultJointIndicesAttributeName() )
            return QStringLiteral("JOINTS_0");
        if( name == Qt3DRender::QAttribute::defaultJointWeightsAttributeName() )
            return QStringLiteral("WEIGHTS_0");
        return name;
    }
}

template<typename T>
std::pair<QString, int> addAttributeToMesh(const Qt3DRender::QAttribute &attribute, draco::Mesh &mesh)
{
    if( attributeTypeFromName(attribute.name()) == draco::GeometryAttribute::Type::POSITION )
        mesh.set_num_points(attribute.count());

    const QByteArray packed_data = packedDataInBuffer<T>(
                attribute.buffer()->data(),
                attribute.vertexSize(),
                attribute.byteOffset(),
                attribute.byteStride(),
                attribute.count());

    draco::PointAttribute mesh_attribute;
    mesh_attribute.Init(
                attributeTypeFromName(attribute.name()),
                nullptr,
                static_cast<qint8>(attribute.vertexSize()),
                toDracoDataType<T>::type,
                false,
                actualStride<T>(attribute.byteStride(), attribute.vertexSize()),
                0);

    auto att_id = mesh.AddAttribute(mesh_attribute, true, attribute.count());
    if( att_id == -1 )
        return { {}, -1 };

    mesh.attribute(att_id)->buffer()->Write(0, packed_data.data(), static_cast<std::size_t>(packed_data.size()));

    return { semanticNameFromAttribute(attribute), att_id };
}

template<typename T>
QVector<draco::PointIndex> createIndicesVectorFromAttribute(const Qt3DRender::QAttribute &attribute)
{
    const auto data = attribute.buffer()->data().mid(int(attribute.byteOffset()));
    const auto attr_count = attribute.count();
    const int stride = actualStride<T>(qint32(attribute.byteStride()), 1);

    Q_ASSERT_X(attr_count < std::numeric_limits<qint32>::max(), "GLTF2::createIndicesVectorFromAttribute", "Too many elements for a QVector");

    QVector<draco::PointIndex> indices_vector(static_cast<int>(attr_count));

    for( qint32 i = 0, n = qint32(attr_count); i < n; ++i )
        indices_vector[i] = *reinterpret_cast<const T *>(data.data() + i * stride);

    return indices_vector;
}

void addFacesToMesh(const QVector<draco::PointIndex> &indices, draco::Mesh &draco_mesh)
{
    for( qint32 i = 0, nb_indices = indices.size() / 3; i < nb_indices; ++i )
        draco_mesh.AddFace({ indices[3 * i + 0], indices[3 * i + 1], indices[3 * i + 2] });
}

template<typename T>
bool compressAttribute(
        const Qt3DRender::QAttribute &attribute,
        draco::Mesh &draco_mesh,
        QMap<QString, int> &attributes)
{
    auto compressed_attr = addAttributeToMesh<T>(attribute, draco_mesh);
    if( compressed_attr.second == -1 ) {
        qCWarning(dracocompressor) << "Could not add attribute to mesh: " << attribute.name();
        return false;
    }
    attributes[compressed_attr.first] = compressed_attr.second;
    return true;
}

} // namespace

SceneEntityParser::CompressedMesh SceneEntityParser::compressMesh(const Qt3DRender::QGeometry *geometry, const QVariantMap &options)
{
    draco::EncoderBuffer *compress_buffer = new draco::EncoderBuffer;
    QMap<QString, int> attributes;
    draco::Mesh draco_mesh;

    // Convert Qt3D geometry to draco mesh
    const auto geometry_attributes = geometry->attributes();
    const Qt3DRender::QAttribute *indices_attribute = nullptr;

    for( const auto *attribute : geometry_attributes ) {
        if( attribute->name() == VERTICES_ATTRIBUTE_NAME ) {
            draco_mesh.set_num_points(attribute->count());
            break;
        }
    }

    for( const auto *attribute : geometry_attributes ) {
        if( attribute->attributeType() == Qt3DRender::QAttribute::IndexAttribute ) {
            indices_attribute = attribute;
            continue;
        }

        switch( attribute->vertexBaseType() ) {
            case Qt3DRender::QAttribute::VertexBaseType::Float: { // Put first for optimisation
                compressAttribute<float>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::Double: { // Put first for optimisation
                compressAttribute<double>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::Byte: {
                static_assert( std::numeric_limits<qint8>::min() < 0, "Platform not supporting signed byte" );
                compressAttribute<qint8>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::UnsignedByte: {
                compressAttribute<quint8>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::Short: {
                compressAttribute<qint16>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::UnsignedShort: {
                compressAttribute<quint16>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::Int: {
                compressAttribute<qint32>(*attribute, draco_mesh, attributes);
                break;
            }
            case Qt3DRender::QAttribute::VertexBaseType::UnsignedInt: {
                compressAttribute<quint32>(*attribute, draco_mesh, attributes);
                break;
            }
            default: {
                qCWarning(dracocompressor) << "Warning: skipping geometry attribute" << attribute->name()
                                           << "of unhandled type" << attribute->vertexBaseType();
                break;
            }
        }
    }

    if( indices_attribute != nullptr ) {
        switch( indices_attribute->vertexBaseType() ) {
            case Qt3DRender::QAttribute::VertexBaseType::Byte:
                addFacesToMesh(createIndicesVectorFromAttribute<qint8>(*indices_attribute), draco_mesh);
                break;
            case Qt3DRender::QAttribute::VertexBaseType::UnsignedByte:
                addFacesToMesh(createIndicesVectorFromAttribute<quint8>(*indices_attribute), draco_mesh);
                break;
            case Qt3DRender::QAttribute::VertexBaseType::Short:
                addFacesToMesh(createIndicesVectorFromAttribute<qint16>(*indices_attribute), draco_mesh);
                break;
            case Qt3DRender::QAttribute::VertexBaseType::UnsignedShort:
                addFacesToMesh(createIndicesVectorFromAttribute<quint16>(*indices_attribute), draco_mesh);
                break;
            case Qt3DRender::QAttribute::VertexBaseType::Int:
                addFacesToMesh(createIndicesVectorFromAttribute<qint32>(*indices_attribute), draco_mesh);
                break;
            case Qt3DRender::QAttribute::VertexBaseType::UnsignedInt:
                addFacesToMesh(createIndicesVectorFromAttribute<quint32>(*indices_attribute), draco_mesh);
                break;
            default: {
                qCWarning(dracocompressor) << "Warning: skipping indices attribute" << indices_attribute->name()
                                           << "of unhandled type" << indices_attribute->vertexBaseType();
                break;
            }
        }
    }

    draco::Encoder encoder;
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, options["draco.quantization.position"].toInt());
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, options["draco.quantization.normal"].toInt());
    encoder.SetAttributeQuantization(draco::GeometryAttribute::COLOR, options["draco.quantization.color"].toInt());
    encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, options["draco.quantization.tex_coord"].toInt());
    encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, options["draco.quantization.generic"].toInt());

    encoder.SetSpeedOptions(options["draco.encoding_speed"].toInt(), options["draco.decoding_speed"].toInt());

    encoder.SetTrackEncodedProperties(true);

    const auto status = encoder.EncodeMeshToBuffer(draco_mesh, compress_buffer);
    if( !status.ok() ) {
        qCWarning(dracocompressor) << status.code() << status.error_msg();
        return {};
    }

    return { compress_buffer, attributes };
}

} // namespace GLTF2
