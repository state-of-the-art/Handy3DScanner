#ifndef DEPTHCAMERA_H
#define DEPTHCAMERA_H

#include <QDebug>
#include <QString>
#include <QImage>

#include <QtQml/qqml.h>
#include <QQmlListProperty>

class PointCloud;

class DepthCamera :
    public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isConnected READ getIsConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(bool isStreaming READ getIsStreaming NOTIFY isStreamingChanged)
    Q_PROPERTY(bool isScanning READ getIsScanning NOTIFY isScanningChanged)
    Q_PROPERTY(QQmlListProperty<PointCloud> pointclouds READ getPointClouds NOTIFY isPointCloudsChanged)
    Q_PROPERTY(QString connectionType READ getConnectionType NOTIFY connectionTypeChanged)
    Q_PROPERTY(qreal streamFPS READ getStreamFPS NOTIFY streamFPSChanged)
    Q_PROPERTY(qreal streamFWT READ getStreamFWT NOTIFY streamFWTChanged)
    Q_PROPERTY(qreal streamFPT READ getStreamFPT NOTIFY streamFPTChanged)

public:
    DepthCamera(const QString& serialNumber);
    bool getIsConnected() const;
    bool getIsStreaming() const;
    bool getIsScanning() const;
    QQmlListProperty<PointCloud> getPointClouds();
    QString getConnectionType() const;
    qreal getStreamFPS() const { return m_stream_fps; }
    qreal getStreamFWT() const { return m_stream_fwt; }
    qreal getStreamFPT() const { return m_stream_fpt; }

    static void declareQML();

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void makeShot() = 0;
    Q_INVOKABLE void loadPointCloud(QString path);

protected slots:
    void setStreamFPS(const qreal fps) { m_stream_fps = fps; emit streamFPSChanged(); }
    void setStreamFWT(const qreal fwt) { m_stream_fwt = fwt; emit streamFWTChanged(); }
    void setStreamFPT(const qreal fpt) { m_stream_fpt = fpt; emit streamFPTChanged(); }

signals:
    void newDepthImage(QImage image);
    void newColorImage(QImage image);
    void isConnectedChanged(const bool isConnected);
    void isStreamingChanged(const bool isStreaming);
    void isScanningChanged(const bool isScanning);
    void isPointCloudsChanged();
    void errorOccurred(const QString &error);
    void connectionTypeChanged();
    void streamFPSChanged();
    void streamFWTChanged();
    void streamFPTChanged();

protected:
    QString m_serialNumber;
    void setIsConnected(const bool isConnected);
    void setIsStreaming(const bool isStreaming);
    void setIsScanning(const bool isScanning);
    void addPointCloud(PointCloud* pc);
    void removePointCloud(PointCloud* pc);
    void setConnectionType(QString connection_type);

private:
    bool m_isConnected;
    bool m_isStreaming;
    bool m_isScanning;
    QList<PointCloud*> m_pointclouds;
    QString m_connectionType;

    qreal m_stream_fps;
    qreal m_stream_fwt;
    qreal m_stream_fpt;
};

#endif // DEPTHCAMERA_H
