#include "depthcamera.h"

#include <QDebug>
#include <QLoggingCategory>

#include "pointcloud.h"
#include "pointcloudexception.h"

Q_LOGGING_CATEGORY(depthcamera, "DepthCamera")

DepthCamera::DepthCamera(const QString& serialNumber):
    m_serialNumber(serialNumber),
    m_isConnected(false),
    m_isStreaming(false),
    m_isScanning(false),
    m_connectionType(""),
    m_stream_fps(0.0),
    m_stream_fwt(0.0),
    m_stream_fpt(0.0)
{
    qCDebug(depthcamera) << "Init DepthCamera done";
}

bool DepthCamera::getIsConnected() const
{
    return m_isConnected;
}

void DepthCamera::setIsConnected(bool isConnected)
{
    if( isConnected != m_isConnected ) {
        m_isConnected = isConnected;
        emit isConnectedChanged(isConnected);
    }
}

bool DepthCamera::getIsStreaming() const
{
    return m_isStreaming;
}

void DepthCamera::setIsStreaming(bool isStreaming)
{
    if( isStreaming != m_isStreaming ) {
        m_isStreaming = isStreaming;
        emit isStreamingChanged(isStreaming);
    }
}

bool DepthCamera::getIsScanning() const
{
    return m_isScanning;
}

void DepthCamera::setIsScanning(bool isScanning)
{
    if( isScanning != m_isScanning ) {
        m_isScanning = isScanning;
        emit isScanningChanged(isScanning);
    }
}

QQmlListProperty<PointCloud> DepthCamera::getPointClouds()
{
    return QQmlListProperty<PointCloud>(this, &m_pointclouds);
}

QString DepthCamera::getConnectionType() const
{
    return m_connectionType;
}

void DepthCamera::declareQML()
{
    qmlRegisterType<PointCloud>("io.stateoftheart.PointCloud", 1, 0, "PointCloud");
}

void DepthCamera::addPointCloud(PointCloud *pc)
{
    m_pointclouds.append(pc);
    emit isPointCloudsChanged();
}

void DepthCamera::removePointCloud(PointCloud *pc)
{
    m_pointclouds.removeOne(pc);
    emit isPointCloudsChanged();
}

void DepthCamera::setConnectionType(QString connection_type)
{
    if( connection_type != m_connectionType ) {
        m_connectionType = connection_type;
        emit connectionTypeChanged();
    }
}

void DepthCamera::loadPointCloud(QString path)
{
    try {
        m_pointclouds.append(PointCloud::loadPCD(path));
        emit isPointCloudsChanged();
    } catch( PointCloudException e ) {
        qCWarning(depthcamera) << e.getMessage();
    }
}
