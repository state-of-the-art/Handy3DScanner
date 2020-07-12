#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <QObject>
#include <QDateTime>
#include <QVector3D>
#include <QQuaternion>

class QFile;

namespace Qt3DRender {
    class QGeometry;
    class QBuffer;
    class QAttribute;
}

class PointCloud : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString filePath READ getFilePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QDateTime shootTime READ getShootTime WRITE setShootTime NOTIFY shootTimeChanged)
    Q_PROPERTY(QVariantMap metadata READ getMetadata NOTIFY metadataChanged)
    Q_PROPERTY(Qt3DRender::QGeometry* geometry READ getGeometry NOTIFY geometryChanged)
    Q_PROPERTY(QQuaternion orientation READ getOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QQuaternion objectOrientation READ getObjectOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QVector3D position READ getPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D objectPosition READ getObjectPosition NOTIFY positionChanged)

public:
    explicit PointCloud(QObject *parent = nullptr);
    ~PointCloud() override;

    static PointCloud* loadPCD(QString filepath);
    Q_INVOKABLE void savePCD(QString dirpath);

    enum PCDFieldType {
        Signed,   // I
        Unsigned, // U
        Float     // F
    };

    // Options > 16bit are reserved and used for the shader intercommunication
    enum Option {              // fedcba98 76543210
        None         = 0x0000, // 00000000 00000000
        Marked       = 0x0001, //                 ^
        Hidden       = 0x0002  //                ^
    };

    QString getName() const;
    void setName(const QString &value);
    QString getFilePath() const;
    void setFilePath(const QString &value);
    QDateTime getShootTime() const;
    void setShootTime(const QDateTime &value);
    QVariantMap getMetadata() const;
    void setMetadata(const QVariantMap &value);
    void addMetadata(const QString &key, const QString &value);
    void deleteMetadata(const QString &key);

    void setPCPoints(const QByteArray &data);
    void setPCColor(const QByteArray &data);

    void setVertexBuffer(const QByteArray &data);
    void setColorBuffer(const QByteArray &data);
    void setIndexNumber(const qint32 number);
    void setIndexBuffer(const QByteArray &data);
    void setOptionsBuffer(const QByteArray &data);

    qint32 getIndexNumber();

    qint32 getPointsNumber() const;
    void setPointsNumber(const qint32 value);

    qint32 getWidth() const;
    void setWidth(const qint32 value);
    qint32 getHeight() const;
    void setHeight(const qint32 value);

    void writePCDBinary(QFile *file) const;
    void writePCDBinaryCompressed(QFile *file) const;

    QVector3D getPosition() const;
    QVector3D getObjectPosition() const;
    void setPosition(QVector3D value);
    QQuaternion getOrientation() const;
    QQuaternion getObjectOrientation() const;
    void setOrientation(QQuaternion value);

    Qt3DRender::QGeometry* getGeometry();

    Q_INVOKABLE qint32 getMeshPointId(int primitive_type, qint32 index_id);
    Q_INVOKABLE qint32 getPointOptions(qint32 mesh_point_id);

    Q_INVOKABLE void switchIndexAttribute(bool enabled);

    // Mark
    Q_INVOKABLE bool isPointMarked(qint32 mesh_point_id);
    Q_INVOKABLE bool isPointsMarked();

    Q_INVOKABLE void markPoint(qint32 mesh_point_id);
    Q_INVOKABLE void markPoints(QList<qint32> mesh_point_ids);

    Q_INVOKABLE void unmarkPoint(qint32 mesh_point_id);
    Q_INVOKABLE void unmarkAll();

    // Points operations
    Q_INVOKABLE void movePoint(qint32 mesh_point_id, QVector3D newpos);

    Q_INVOKABLE void deletePoint(qint32 mesh_point_id);
    Q_INVOKABLE void deleteMarkedPoints();
    Q_INVOKABLE void deleteUnmarkedPoints();

    Q_INVOKABLE void cropToSelection();

signals:
    void nameChanged();
    void filePathChanged();
    void shootTimeChanged();
    void metadataChanged();
    void geometryChanged();
    void positionChanged();
    void orientationChanged();

public slots:
    void buildMeshFromPC();

protected:
    void setVertexAttribute();
    void setColorAttribute();
    void setIndexAttribute();
    void setOptionsAttribute();

    void filterBadZPoints();
    void generateIndexes();
    QByteArray getMeshColors();

    QVector3D getPoint(int index);
    float getPointZ(int index);

    inline qint32 meshPointToPC(qint32 mesh_point_id);
    inline qint32 pcPointToMesh(qint32 pc_point_id);

private:
    const static QString METADATA_PREFIX;

    QString name;
    QString file_path;
    QDateTime shoot_time;
    QVariantMap metadata;

    qint32 points_number;
    qint32 width;
    qint32 height;

    QVector3D *position;
    QQuaternion *orientation;

    // Original point cloud data
    QByteArray pc_points;
    QByteArray pc_colors;
    QVector<qint32> mesh_to_pc_index;
    QVector<qint32> pc_to_mesh_index;

    // Current display mesh info
    bool is_geometry_built;
    qint32 mesh_points_number;
    Qt3DRender::QGeometry *geometry;

    Qt3DRender::QBuffer *vertex_buffer;
    Qt3DRender::QAttribute *vertex_attribute;
    quint8 vertex_format;

    Qt3DRender::QBuffer *color_buffer;
    Qt3DRender::QAttribute *color_attribute;
    quint8 color_format;

    Qt3DRender::QBuffer *index_buffer;
    Qt3DRender::QAttribute *index_attribute;
    qint32 index_number;
    bool index_enabled; // To improve performance of showing pointcloud as points & show all the points that not in the index

    Qt3DRender::QBuffer *options_buffer;
    Qt3DRender::QAttribute *options_attribute;
};

#endif // POINTCLOUD_H
