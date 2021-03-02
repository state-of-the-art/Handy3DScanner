#ifndef POINTCLOUDDATA_H
#define POINTCLOUDDATA_H

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QVariantMap>

/**
 * Point Cloud Data container
 */
class PointCloudData
{
public:
    PointCloudData() {}
    ~PointCloudData() {}

    QString name;
    QString file_path;
    QDateTime shoot_time;
    QVariantMap metadata;

    quint32 points_number;
    quint32 width;
    quint32 height;

    QByteArray points;
    QByteArray colors;
};

#endif // POINTCLOUDDATA_H
