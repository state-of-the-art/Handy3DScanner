#include "pointcloud.h"

#include <QFile>
#include <QFileInfo>
#include <QtMath>
#include <QLoggingCategory>

#include "pointcloudexception.h"
#include "src/settings.h"
#include "lib/pcl/lzf.h"

#ifdef ANDROID
#include "src/androidwrapper.h"
#endif

#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>

Q_LOGGING_CATEGORY(pc, "PointCloud")

using namespace Qt3DRender;

const QString PointCloud::METADATA_PREFIX = "# h3ds.";

PointCloud::PointCloud(QObject *parent) :
    QObject(parent),
    name(),
    file_path(),
    shoot_time(),
    metadata(),
    points_number(0),
    width(0),
    height(0),
    position(new QVector3D()),
    orientation(new QQuaternion()),
    is_geometry_built(false),
    mesh_points_number(0),
    geometry(new QGeometry()),
    vertex_buffer(new QBuffer(QBuffer::VertexBuffer, geometry)),
    vertex_attribute(new QAttribute()),
    vertex_format(QAttribute::Float),
    color_buffer(new QBuffer(QBuffer::VertexBuffer, geometry)),
    color_attribute(new QAttribute()),
    color_format(QAttribute::UnsignedByte),
    index_buffer(new QBuffer(QBuffer::IndexBuffer, geometry)),
    index_attribute(new QAttribute()),
    index_number(0),
    index_enabled(Settings::I()->val("UI.Edit.display_mode").toString() != QLatin1Literal("POINTS")),
    options_buffer(new QBuffer(QBuffer::VertexBuffer, geometry)),
    options_attribute(new QAttribute())
{
    qCDebug(pc) << "Creating empty PointCloud";
}

PointCloud::~PointCloud()
{
    qCDebug(pc) << "Destroying PointCloud" << getName();
    delete options_attribute;
    delete options_buffer;
    delete index_attribute;
    delete index_buffer;
    delete color_attribute;
    delete color_buffer;
    delete vertex_attribute;
    delete vertex_buffer;
    delete geometry;
}

PointCloud* PointCloud::loadPCD(QString filepath)
{
    qCDebug(pc) << "Building PointCloud out of PCD file" << filepath;
#ifndef ANDROID
    QFile file(filepath);
    if( ! file.exists() )
        throw PointCloudException(QString("Unable to load PCD due to file is not existing: %1").arg(filepath));
    file.open(QIODevice::ReadOnly);
#else
    QFile file;
    int fd = AndroidWrapper::getFileDescriptor(filepath);
    if( fd < 0 )
        throw PointCloudException(QString("Unable to load PCD due to file is not available: %1").arg(filepath));
    file.open(fd, QFile::ReadOnly);
#endif
    if( ! file.isReadable() )
        throw PointCloudException(QString("Unable to load PCD due to file is not readable: %1").arg(filepath));

    PointCloud *out = new PointCloud();
    out->setFilePath(filepath);

    // Reading header
    int fields_number = 0;
    QStringList fields_order;
    QList<quint16> fields_size;
    QList<PCDFieldType> fields_type;
    QList<quint16> fields_count;
    QString data_format;

    while( true ) {
        QString line = file.readLine().trimmed();
        if( line.startsWith('#') ) {
            if( ! line.startsWith(PointCloud::METADATA_PREFIX) )
                continue;

            QString key, value;
            {
                line = line.mid(PointCloud::METADATA_PREFIX.size());
                int sep_pos = line.indexOf(": ");
                key = line.left(sep_pos);
                value = line.mid(sep_pos+2);
            }
            if( key == "name" )
                out->setName(value);
            else if( key == "shootTime" )
                out->setShootTime(QDateTime::fromSecsSinceEpoch(value.toLongLong()));
            else {
                out->addMetadata(key, value);
            }
            continue;
        }
        QStringList data = line.split(" ");
        if( data.first() == "VERSION" ) {
            if( data[1] != "0.7" )
                throw PointCloudException(QString("Unsupported PCD version %1").arg(data[1]));
        }
        else if( data.first() == "FIELDS" ) {
            for( quint8 i = 1; i < data.size(); i++ )
                fields_order += data[i];
            if( fields_number == 0 )
                fields_number = fields_order.count();
        }
        else if( data.first() == "SIZE" ) {
            for( quint8 i = 1; i < data.size(); i++ ) {
                fields_size += data[i].toUShort();
            }
            if( fields_number == 0 )
                fields_number = fields_size.count();
        }
        else if( data.first() == "TYPE" ) {
            for( quint8 i = 1; i < data.size(); i++ ) {
                switch( data[i].data()[0].toLatin1() ) {
                case 'I':
                    fields_type += PCDFieldType::Signed;
                    break;
                case 'U':
                    fields_type += PCDFieldType::Unsigned;
                    break;
                case 'F':
                    fields_type += PCDFieldType::Float;
                    break;
                }
            }
            if( fields_number == 0 )
                fields_number = fields_type.count();
        }
        else if( data.first() == "COUNT" ) {
            for( quint8 i = 1; i < data.size(); i++ ) {
                fields_count += data[i].toUShort();
                if( fields_count[i-1] != 1 )
                    throw PointCloudException(QString("Unsupported more than 1 count per field %1").arg(line));
            }
            if( fields_number == 0 )
                fields_number = fields_count.count();
        }
        else if( data.first() == "WIDTH" ) {
            out->setWidth(data[1].toUShort());
        }
        else if( data.first() == "HEIGHT" ) {
            out->setHeight(data[1].toUShort());
        }
        else if( data.first() == "VIEWPOINT" ) {
            // tx ty tz
            out->setPosition(QVector3D(data[1].toFloat(), data[2].toFloat(), data[3].toFloat()));
            // qw qx qy qz
            out->setOrientation(QQuaternion(data[4].toFloat(), data[5].toFloat(), data[6].toFloat(), data[7].toFloat()));
        }
        else if( data.first() == "POINTS" ) {
            out->setPointsNumber(data[1].toInt());
        }
        else if( data.first() == "DATA" ) {
            data_format = data[1];
            break;
        } else
            throw PointCloudException(QString("Unsupported PCD header key %1").arg(data.first()));
    }

    if( out->getName().isEmpty() )
        out->setName(QFileInfo(file).completeBaseName());

    if( ! out->getShootTime().isValid() )
        out->setShootTime(file.fileTime(QFile::FileBirthTime));
    if( ! out->getShootTime().isValid() )
        out->setShootTime(file.fileTime(QFile::FileModificationTime));

    if( fields_number == 0 || fields_order.count() == 0 || fields_count.count() == 0 || fields_size.count() == 0 || fields_type.count() == 0 )
        throw PointCloudException(QString("Not found fields specifications for order, count, size or type").arg(fields_number));

    if( fields_number != fields_order.count() ||
        fields_number != fields_count.count() ||
        fields_number != fields_size.count() ||
        fields_number != fields_type.count() )
        throw PointCloudException(QString("Not equal number of fields order, count, size or type (should be %3)").arg(fields_number));

    // Reading data
    QByteArray vertex_buffer;
    QByteArray color_buffer;

    qint32 color_offset = -1;

    if( data_format == "binary" ) {
        // Counting offsets and block size
        // TODO: expecting x y z rgb, where x y and z can't be separated...
        quint8 data_buffer_block_size = 0;
        {
            qint8 offset_counter = 0;
            for( quint8 i = 0; i < fields_number; i++ ) {
                // Color offset
                if( fields_order[i] == "rgb" )
                    color_offset = offset_counter;
                offset_counter += fields_size[i];

                // Block size - rgb will be converted to multiple of 3
                data_buffer_block_size += fields_order[i] == "rgb" ? fields_size[i] / 3 * 3 : fields_size[i];
            }
        }

        qCDebug(pc) << "Buffer block size:" << data_buffer_block_size;
        qCDebug(pc) << "Color offset:" << color_offset;
        qCDebug(pc) << "Points number:" << out->getPointsNumber();

        // TODO: set vertex_format & color_format based on size & type

        // If pc contains color we need to read rgb as bgr due to bigendian
        if( color_offset > -1 ) {
            quint16 color_size = fields_size[fields_order.indexOf("rgb")];
            QByteArray bgr;
            for( qint32 p = 0 ; p < out->getPointsNumber(); p++ ) {
                // Reading bytes before rgb
                vertex_buffer += file.read(color_offset);
                // Reading color as bgr
                bgr = file.read(color_size);
                color_buffer += bgr[2];
                color_buffer += bgr[1];
                color_buffer += bgr[0];
                color_buffer += '\xff';
            }
        } else
            vertex_buffer = file.read(out->getPointsNumber() * data_buffer_block_size);

        /*if( static_cast<qint32>(vertex_buffer.size() + color_buffer.size()) != data_buffer_block_size * out->getPointsNumber() )
            throw PointCloudException(QString("PCD data doesn't contain required number of required bytes: %1 != %2 + %3")
                                      .arg(data_buffer_block_size * out->getPointsNumber()).arg(vertex_buffer.size()).arg(color_buffer.size()));*/
    }
    else if( data_format == "binary_compressed" ) {
        // Reading to compressed buffer
        quint32 compressed_size;
        quint32 data_size;
        memcpy(&compressed_size, file.read(qint8(sizeof(quint32))).constData(), qint8(sizeof(quint32)));
        memcpy(&data_size, file.read(qint8(sizeof(quint32))).constData(), qint8(sizeof(quint32)));

        qCDebug(pc) << "Data size:" << data_size << "Compressed:" << compressed_size;

        // Decompressing the data
        QByteArray to_decompress = file.read(compressed_size);
        if( quint32(to_decompress.size()) != compressed_size )
            throw PointCloudException(QString("Not enough compressed data in the file: %1 (expected %2)").arg(to_decompress.size()).arg(compressed_size));
        QByteArray data(data_size, Qt::Uninitialized);
        if( data_size != pcl::lzfDecompress(to_decompress.data(), compressed_size, data.data(), data_size) )
            throw PointCloudException("Error during decompression the data");

        // Reading the xxyyzz vertex data
        qint32 y_offset = out->getPointsNumber() * qint8(sizeof(float));
        qint32 z_offset = y_offset*2;
        for( qint32 p = 0; p < out->getPointsNumber(); p++ ) {
            vertex_buffer += data.mid(p*4, 4);
            vertex_buffer += data.mid(p*4+y_offset, 4);
            vertex_buffer += data.mid(p*4+z_offset, 4);
        }

        // Reading color data
        if( fields_order.contains("rgb") ) {
            color_offset = vertex_buffer.size();
            quint16 color_size = fields_size[fields_order.indexOf("rgb")];
            for( qint32 p = 0 ; p < out->getPointsNumber(); p++ ) {
                color_buffer += data[color_offset+p*color_size+2];
                color_buffer += data[color_offset+p*color_size+1];
                color_buffer += data[color_offset+p*color_size];
                color_buffer += '\xff';
            }
        }
    } else
        throw PointCloudException(QString("Unsupported PCD data format %1").arg(data_format));

    qCDebug(pc) << "Vertex data size:" << vertex_buffer.size();
    qCDebug(pc) << "Color data size:" << color_buffer.size();

    out->setPCPoints(vertex_buffer);
    if( color_offset > -1 )
        out->setPCColor(color_buffer);

    return out;
}

void PointCloud::savePCD(QString dirpath)
{
    qCDebug(pc) << "Saving Point Cloud" << getName() << "to directory" << dirpath;
#ifndef ANDROID
    QString filepath = dirpath+"/"+getName()+".pcd";
    QFile file(filepath);
    file.open(QIODevice::WriteOnly);
    if( !file.isWritable() )
        throw PointCloudException(QString("Unable to write PCD due to file is not writable: %1").arg(filepath));
#else
    QFile file;
    int fd = AndroidWrapper::getFileTreeDescriptor(dirpath, getName()+".pcd", false);
    if( fd < 0 )
        throw PointCloudException(QString("Unable to write PCD due to file descriptor is not valid: %1").arg(dirpath));
    file.open(fd, QFile::WriteOnly);
#endif

    // Writing header
    bool rgb_available = pc_colors.size() > 0;
    QString header;
    header.append("# .PCD v0.7 - Point Cloud Data file format\n");
    header.append(QString(PointCloud::METADATA_PREFIX).append("version: %1\n").arg(PROJECT_VERSION));
    header.append(QString(PointCloud::METADATA_PREFIX).append("name: %1\n").arg(getName()));
    header.append(QString(PointCloud::METADATA_PREFIX).append("shootTime: %1\n").arg(getShootTime().toSecsSinceEpoch()));

    // Attaching various metadata
    for( QString key : metadata.keys() ) {
        if( key == "version" )
            header.append(QString(PointCloud::METADATA_PREFIX).append("capture_version: %2\n").arg(metadata[key].toString()));
        else
            header.append(QString(PointCloud::METADATA_PREFIX).append("%1: %2\n").arg(key).arg(metadata[key].toString()));
    }

    // Attaching header PCD info
    header.append(QString("VERSION 0.7\n"));
    header.append(QString("FIELDS x y z%1\n").arg(rgb_available ? " rgb" : ""));
    header.append(QString("SIZE 4 4 4%1\n").arg(rgb_available ? " 4" : ""));
    header.append(QString("TYPE F F F%1\n").arg(rgb_available ? " U" : ""));
    header.append(QString("COUNT 1 1 1%1\n").arg(rgb_available ? " 1" : ""));
    header.append(QString("WIDTH %1\n").arg(getWidth()));
    header.append(QString("HEIGHT %1\n").arg(getHeight()));
    header.append(QString("VIEWPOINT %1 %2 %3 %4 %5 %6 %7\n").arg(getPosition().x()).arg(getPosition().y()).arg(getPosition().z())
            .arg(getOrientation().scalar()).arg(getOrientation().x()).arg(getOrientation().y()).arg(getOrientation().z()));
    header.append(QString("POINTS %1\n").arg(points_number));
    file.write(header.toLatin1());

    if( Settings::I()->val("Save.PCD.compress_file").toBool() )
        writePCDBinaryCompressed(&file);
    else
        writePCDBinary(&file);

    file.flush();
    file.close();
}

void PointCloud::writePCDBinary(QFile *file) const
{
    bool rgb_available = pc_colors.size() > 0;
    file->write("DATA binary\n");

    // TODO: probably issues with bigendian - use QEndian to fix that.
    for( qint32 i = 0; i < points_number; ++i ) {
        // Writing vertices
        file->write(pc_points.mid(i*3*4, 3*4)); // X Y Z

        if( ! rgb_available )
            continue;

        // Getting point rgb color - in PCD it's encoded as bgr due to BigEndian
        file->write(pc_colors.mid(i*4+2, 1)); // Blue
        file->write(pc_colors.mid(i*4+1, 1)); // Green
        file->write(pc_colors.mid(i*4, 1)); // Red
        file->write(pc_colors.mid(i*4+3, 1)); // Alpha
    }
}

void PointCloud::writePCDBinaryCompressed(QFile *file) const
{
    file->write("DATA binary_compressed\n");

    // TODO: support something other than float for vertexes
    int vertices_size = points_number * 3*4;
    quint32 data_size = vertices_size + points_number * 4;
    if( data_size * 3 / 2 > std::numeric_limits<qint32>::max() )
        qCWarning(pc) << "The input data exceeds the maximum size for compressed version 0.7 pcds of"
                      << static_cast<size_t>( std::numeric_limits<qint32>::max() ) * 2 / 3 << " bytes";

    // TODO: it's ok if there is less than 156 pointclouds 1280x720 (float points + rgb 3 bytes)
    QByteArray to_compress(data_size, Qt::Uninitialized);

    // Preparing the xxyyzz vertex data
    // TODO: probably issues with bigendian - use QEndian to fix that.
    for( quint8 i = 0; i < 3; i++ ) {
        for( qint32 j = 0; j < points_number; j++ )
            to_compress.replace(i*points_number*4+j*4, 4, pc_points.mid(i*4+j*4*3, 4));
    }

    for( int c = 0; c < points_number; c++ ) {
        to_compress.replace(vertices_size+c*4, 1, pc_colors.mid(c*4+2,1)); // Blue
        to_compress.replace(vertices_size+c*4+1, 1, pc_colors.mid(c*4+1,1)); // Green
        to_compress.replace(vertices_size+c*4+2, 1, pc_colors.mid(c*4,1)); // Red
        to_compress.replace(vertices_size+c*4+3, 1, pc_colors.mid(c*4+3,1)); // Alpha
    }

    // Compress the data
    QByteArray temp_buf(data_size * 3 / 2 + 8, Qt::Uninitialized);
    quint32 compressed_size = pcl::lzfCompress(to_compress.data(), data_size, temp_buf.data(), data_size * 3 / 2);
    if( compressed_size == 0 ) {
        qCWarning(pc, "Compression failed");
        return;
    }

    qCDebug(pc) << "Data size:" << data_size << QByteArray(reinterpret_cast<const char *>(&data_size), 4).toHex()
             << "Compressed:" << compressed_size << QByteArray(reinterpret_cast<const char *>(&compressed_size), 4).toHex();

    temp_buf.prepend(reinterpret_cast<const char *>(&data_size), qint8(sizeof(quint32)));
    temp_buf.prepend(reinterpret_cast<const char *>(&compressed_size), qint8(sizeof(quint32)));
    temp_buf.resize(compressed_size + 8);

    file->write(temp_buf);
}

void PointCloud::setVertexBuffer(const QByteArray &data)
{
    for( QAttribute *attr : geometry->attributes() ) {
        if( attr->name() == QAttribute::defaultPositionAttributeName() )
            geometry->removeAttribute(attr);
    }

    vertex_buffer->setData(data);

    setVertexAttribute();

    emit geometryChanged();
}

void PointCloud::setColorBuffer(const QByteArray &data)
{
    for( QAttribute *attr : geometry->attributes() ) {
        if( attr->name() == QAttribute::defaultColorAttributeName() )
            geometry->removeAttribute(attr);
    }

    color_buffer->setData(data);

    setColorAttribute();

    emit geometryChanged();
}

void PointCloud::setIndexNumber(const qint32 number)
{
    index_number = number;
}

void PointCloud::setIndexBuffer(const QByteArray &data)
{
    index_buffer->setData(data);

    setIndexAttribute();

    switchIndexAttribute(index_enabled);

    emit geometryChanged();
}

void PointCloud::setOptionsBuffer(const QByteArray &data)
{
    for( QAttribute *attr : geometry->attributes() ) {
        if( attr->name() == "vertexOptions" )
            geometry->removeAttribute(attr);
    }

    options_buffer->setData(data);

    setOptionsAttribute();

    emit geometryChanged();
}

qint32 PointCloud::getPointsNumber() const
{
    return points_number;
}

void PointCloud::setPointsNumber(const qint32 value)
{
    points_number = value;

    // Init options buffer
    QByteArray buf(points_number*qint8(sizeof(qint32)), 0x00);
    setOptionsBuffer(buf);
}

void PointCloud::setVertexAttribute()
{
    if( getPointsNumber() == 0 )
        throw PointCloudException(QString("No points number is set"));

    vertex_attribute->setAttributeType(QAttribute::VertexAttribute);
    vertex_attribute->setBuffer(vertex_buffer);
    vertex_attribute->setDataType(static_cast<QAttribute::VertexBaseType>(vertex_format));
    vertex_attribute->setDataSize(3);
    vertex_attribute->setByteOffset(0);
    vertex_attribute->setByteStride(3 * sizeof(float)); // TODO: Only float supported for now
    vertex_attribute->setCount(mesh_points_number);
    vertex_attribute->setName(QAttribute::defaultPositionAttributeName());

    geometry->addAttribute(vertex_attribute);
}

void PointCloud::setColorAttribute()
{
    if( getPointsNumber() == 0 )
        throw PointCloudException(QString("No points number is set"));

    color_attribute->setAttributeType(QAttribute::VertexAttribute);
    color_attribute->setBuffer(color_buffer);
    color_attribute->setDataType(static_cast<QAttribute::VertexBaseType>(color_format));
    color_attribute->setDataSize(4);
    color_attribute->setByteOffset(0);
    color_attribute->setByteStride(4 * sizeof(char)); // TODO: Only byte supported for now
    color_attribute->setCount(mesh_points_number);
    color_attribute->setName(QAttribute::defaultColorAttributeName());

    geometry->addAttribute(color_attribute);
}

void PointCloud::setIndexAttribute()
{
    if( index_number < 1 )
        return;

    index_attribute->setAttributeType(QAttribute::IndexAttribute);
    index_attribute->setBuffer(index_buffer);
    index_attribute->setDataType(QAttribute::UnsignedInt);
    index_attribute->setDataSize(1);
    index_attribute->setByteOffset(0);
    index_attribute->setByteStride(0);
    index_attribute->setCount(index_number);
    index_attribute->setName("index");
}

void PointCloud::setOptionsAttribute()
{
    if( getPointsNumber() == 0 )
        throw PointCloudException(QString("No points number is set"));

    options_attribute->setAttributeType(QAttribute::VertexAttribute);
    options_attribute->setBuffer(options_buffer);
    options_attribute->setDataType(QAttribute::Int);
    options_attribute->setDataSize(1);
    options_attribute->setByteOffset(0);
    options_attribute->setByteStride(1 * sizeof(qint32));
    options_attribute->setCount(mesh_points_number);
    options_attribute->setName("vertexOptions");

    geometry->addAttribute(options_attribute);
}


QByteArray PointCloud::getMeshColors()
{
    qCDebug(pc, "Getting colors for mesh points");
    QByteArray colors;
    colors.reserve(mesh_to_pc_index.size()*4);

    for( qint32 index : mesh_to_pc_index )
        colors.append(pc_colors.mid(index*4, 4));

    colors.resize(mesh_to_pc_index.size()*4);

    return colors;
}

QVector3D PointCloud::getPoint(int index)
{
    QVector3D point;
    const char *raw_data;
    const float *value;

    // TODO: supports only float
    raw_data = &(vertex_buffer->data().constData()[index*3*4]);
    value = reinterpret_cast<const float*>(raw_data);
    point.setX(*value);
    raw_data = &(vertex_buffer->data().constData()[index*3*4+1*4]);
    value = reinterpret_cast<const float*>(raw_data);
    point.setY(*value);
    raw_data = &(vertex_buffer->data().constData()[index*3*4+2*4]);
    value = reinterpret_cast<const float*>(raw_data);
    point.setZ(*value);

    return point;
}

float PointCloud::getPointZ(int index)
{
    const char *raw_data = &(vertex_buffer->data().constData()[index*3*4+2*4]);
    const float *value = reinterpret_cast<const float*>(raw_data);

    return *value;
}

qint32 PointCloud::meshPointToPC(qint32 mesh_point_id)
{
    return mesh_to_pc_index[mesh_point_id];
}

qint32 PointCloud::pcPointToMesh(qint32 pc_point_id)
{
    return pc_to_mesh_index[pc_point_id];
}

QVariantMap PointCloud::getMetadata() const
{
    return metadata;
}

void PointCloud::setMetadata(const QVariantMap &value)
{
    metadata = value;
    emit metadataChanged();
}

void PointCloud::addMetadata(const QString &key, const QString &value)
{
    metadata[key] = value;
    emit metadataChanged();
}

void PointCloud::deleteMetadata(const QString &key)
{
    metadata.remove(key);
    emit metadataChanged();
}

void PointCloud::setPCPoints(const QByteArray &data)
{
    pc_points = data;
}

void PointCloud::setPCColor(const QByteArray &data)
{
    pc_colors = data;
}

QString PointCloud::getName() const
{
    return name;
}

void PointCloud::setName(const QString &value)
{
    name = value;
    emit nameChanged();
}

QString PointCloud::getFilePath() const
{
    return file_path;
}

void PointCloud::setFilePath(const QString &value)
{
    file_path = value;
    emit filePathChanged();
}

QDateTime PointCloud::getShootTime() const
{
    return shoot_time;
}

void PointCloud::setShootTime(const QDateTime &value)
{
    shoot_time = value;
    emit shootTimeChanged();
}

Qt3DRender::QGeometry* PointCloud::getGeometry()
{
    qCDebug(pc) << "Getting geometry for" << name;
    if( !is_geometry_built )
        buildMeshFromPC();
    return geometry;
}

qint32 PointCloud::getMeshPointId(int primitive_type, qint32 index)
{
    if( primitive_type == 0 )
        index = (index+1) * 3;
    return reinterpret_cast<const qint32*>(index_buffer->data().constData())[index];
}

void PointCloud::markPoint(qint32 mesh_point_id)
{
    qint32 val = getPointOptions(mesh_point_id) | Option::Marked;
    options_buffer->updateData(mesh_point_id*4, QByteArray(reinterpret_cast<const char *>(&val), 4));
}

void PointCloud::unmarkPoint(qint32 mesh_point_id)
{
    qint32 val = getPointOptions(mesh_point_id) & (0xffff ^ Option::Marked);
    options_buffer->updateData(mesh_point_id*4, QByteArray(reinterpret_cast<const char *>(&val), 4));
}

void PointCloud::markPoints(QList<qint32> mesh_point_ids)
{
    qint32 val = Option::Marked;
    QByteArray buf = QByteArray(reinterpret_cast<const char *>(&val), 4);
    for( int index : mesh_point_ids ) {
        options_buffer->updateData(index*4, buf);
    }
}

bool PointCloud::isPointMarked(int mesh_point_id)
{
    return (getPointOptions(mesh_point_id) & Option::Marked) == Option::Marked;
}

bool PointCloud::isPointsMarked()
{
    for( int i = 0; i < mesh_points_number; i++ ) {
        if( isPointMarked(i) )
            return true;
    }
    return false;
}

void PointCloud::unmarkAll()
{
    for( int i = 0; i < mesh_points_number; i++ )
        unmarkPoint(i);
}

void PointCloud::movePoint(qint32 mesh_point_id, QVector3D newpos)
{
    float data[3] = { newpos.x(), newpos.y(), newpos.z() };
    pc_points.replace(meshPointToPC(mesh_point_id)*3*4, 3*4, reinterpret_cast<const char*>(&data));
}

void PointCloud::deletePoint(int mesh_point_id)
{
    movePoint(mesh_point_id, QVector3D(0.0f, 0.0f, 0.0f));
}

void PointCloud::deleteMarkedPoints()
{
    for( int i = 0; i < mesh_points_number; i++ ) {
        if( isPointMarked(i) )
            deletePoint(i);
    }
    buildMeshFromPC();
    emit geometryChanged();
}

void PointCloud::deleteUnmarkedPoints()
{
    for( int i = 0; i < mesh_points_number; i++ ) {
        if( ! isPointMarked(i) )
            deletePoint(i);
    }
    buildMeshFromPC();
    emit geometryChanged();
}

qint32 PointCloud::getPointOptions(qint32 mesh_point_id)
{
    const char *raw_data = &(options_buffer->data().constData()[mesh_point_id*4]);
    const qint32 *value = reinterpret_cast<const qint32*>(raw_data);
    return *value;
}

void PointCloud::switchIndexAttribute(bool enabled)
{
    qCDebug(pc) << "Switch index attribute" << enabled;
    index_enabled = enabled;
    if( index_enabled ) {
        qCDebug(pc) << "Enabling indexes" << geometry->attributes().contains(index_attribute);
        if( !geometry->attributes().contains(index_attribute) )
            geometry->addAttribute(index_attribute);
    } else {
        for( QAttribute *attr : geometry->attributes() ) {
            if( attr->attributeType() == QAttribute::IndexAttribute )
                geometry->removeAttribute(attr);
        }
    }
}

void PointCloud::cropToSelection()
{
    // TODO: mesh->pointcloud
    // Find boundaries
    int top = height,
        left = width,
        bottom = 0,
        right = 0;

    // Going from left top corner and update top & left boundaries
    for( int y = 0; y < height; y++ ) {
        for( int x = 0; x < left; x++ ) {
            if( ! isPointMarked(x + y*width) )
                continue;
            qCDebug(pc) << "Found lefttop marked:" << x << y << x + y*width;
            if( x < left )
                left = x;
            if( y < top )
                top = y;
        }
    }

    // Going from right bottom corner and update bottom & right boundaries
    for( int y = height; y >= 0; y-- ) {
        for( int x = width; x > right; x-- ) {
            if( ! isPointMarked(x + y*width) )
                continue;
            qCDebug(pc) << "Found rightbottom marked:" << x << y << x + y*width;
            if( x > right )
                right = x;
            if( y > bottom )
                bottom = y;
        }
    }

    qCDebug(pc) << "Crop: lefttop:" << left << top << "rightbottom:" << right << bottom;

    // Modify the data
    int old_width = width;
    width = right - left;
    height = bottom - top;
    points_number = width * height;

    // Fill buffers with new data
    QByteArray new_vert_data;
    QByteArray new_options_data;
    for( int y = top; y <= bottom; y++ ) {
        new_vert_data += vertex_buffer->data().mid((y*old_width+left)*3*4, width*3*4);
        new_options_data += options_buffer->data().mid((y*old_width+left)*4, width*4);
    }
    setVertexBuffer(new_vert_data);
    setOptionsBuffer(new_options_data);

    if( ! color_buffer->data().isEmpty() ) {
        QByteArray new_color_data;
        for( int y = top; y <= bottom; y++ )
            new_color_data += color_buffer->data().mid((y*old_width+left)*4, width*4);
        setColorBuffer(new_color_data);
    }
}

void PointCloud::filterBadZPoints()
{
    qCDebug(pc, "Filtering bad Z points");

    mesh_to_pc_index.clear();
    mesh_to_pc_index.reserve(points_number);
    pc_to_mesh_index.clear();
    pc_to_mesh_index.reserve(points_number);

    mesh_points_number = points_number;

    const float *raw_points = reinterpret_cast<const float*>(pc_points.constData());
    float z_value;

    qint32 index_offset(0);

    QByteArray new_points(pc_points.size(), Qt::Uninitialized);

    float min_z = float(Settings::I()->val("UI.Edit.PointCloud.point_min_invalid_z").toDouble());
    float max_z = float(Settings::I()->val("UI.Edit.PointCloud.point_max_invalid_z").toDouble());

    for( qint32 i = 0; i < mesh_points_number; ++i ) {
        z_value = raw_points[i*3+2];
        if( z_value > min_z && z_value < max_z ) {
            new_points.replace((i - index_offset)*3*4, 3*4, pc_points.mid(i*3*4, 3*4));
            mesh_to_pc_index.append(i);
            pc_to_mesh_index.append(i - index_offset);
        } else {
            index_offset++;
            pc_to_mesh_index.append(-1);
        }
    }

    qCDebug(pc) << "PCTOMESH:" << pc_to_mesh_index.size() << "MESHTOPC:" << mesh_to_pc_index.size();

    new_points.resize(pc_points.size()-(index_offset*3*4));

    qCDebug(pc) << "PC Points buffer Size:" << pc_points.size() << "Become:" << new_points.size() << "Filtered points:" << index_offset;
    mesh_points_number = mesh_to_pc_index.size();

    setVertexBuffer(new_points);
}

void PointCloud::generateIndexes()
{
    qCDebug(pc, "Generating indexes for pointcloud");

    // Allocating enough memory to store all the possible triangles
    // (height-1)*(width-1) - number of faces
    // 2 - number of tris in a face
    // 3 - points to use in tris
    QByteArray new_indexes((height-1)*(width-1)*2*3*qint8(sizeof(qint32)), Qt::Uninitialized);
    qCDebug(pc) << "Allocated indexes buffer:" << new_indexes.size();

    qint32 *raw_indexes = reinterpret_cast<qint32*>(new_indexes.data());

    qint32 pc_idx;
    for( qint32 y = 0; y < height-1; ++y ) {
        for( qint32 x = 0; x < width-1; ++x ) {
            pc_idx = y*width+x;

            // Check points contains invalid z value
            if( pc_to_mesh_index[pc_idx+1] < 0 || pc_to_mesh_index[pc_idx+width] < 0 ) // Common top right && Common bottom left
                continue;

            // Creating 2 triangle to fill the square of points
            if( pc_to_mesh_index[pc_idx] > 0 ) { // Top left
                raw_indexes[index_number++] = pcPointToMesh(pc_idx);
                raw_indexes[index_number++] = pcPointToMesh(pc_idx+width);
                raw_indexes[index_number++] = pcPointToMesh(pc_idx+1);
            }
            if( pc_to_mesh_index[pc_idx+width+1] > 0 ) { // Bottom right
                raw_indexes[index_number++] = pcPointToMesh(pc_idx+1);
                raw_indexes[index_number++] = pcPointToMesh(pc_idx+width);
                raw_indexes[index_number++] = pcPointToMesh(pc_idx+width+1);
            }
        }
    }
    new_indexes.resize(index_number*qint8(sizeof(qint32)));
    qCDebug(pc) << "Generated indexes:" << index_number << "buffer resized:" << new_indexes.size();

    setIndexBuffer(new_indexes);
    qCDebug(pc, "Generation of indexes done");
}

void PointCloud::buildMeshFromPC()
{
    qCDebug(pc, "Building mesh from pointcloud");

    filterBadZPoints();
    generateIndexes();

    if( pc_colors.size() > 0 )
        setColorBuffer(getMeshColors());
    else {
        // TODO: set better color based on some parameter
        QByteArray color;
        color.fill(char(0xff), mesh_points_number*4);
        setColorBuffer(color);
    }
    is_geometry_built = true;
}

QQuaternion PointCloud::getOrientation() const
{
    return QQuaternion(*orientation);
}

QQuaternion PointCloud::getObjectOrientation() const
{
    return QQuaternion(*orientation).inverted();
}

void PointCloud::setOrientation(QQuaternion value)
{
    delete orientation;
    orientation = new QQuaternion(value);
    emit orientationChanged();
}

QVector3D PointCloud::getObjectPosition() const
{
    return -QVector3D(*position);
}

QVector3D PointCloud::getPosition() const
{
    return QVector3D(*position);
}

void PointCloud::setPosition(QVector3D value)
{
    delete position;
    position = new QVector3D(value);
    emit positionChanged();
}

qint32 PointCloud::getHeight() const
{
    return height;
}

void PointCloud::setHeight(const qint32 value)
{
    height = value;
}

qint32 PointCloud::getWidth() const
{
    return width;
}

void PointCloud::setWidth(const qint32 value)
{
    width = value;
}
