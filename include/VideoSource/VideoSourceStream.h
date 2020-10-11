#ifndef VIDEOSOURCESTREAM_H
#define VIDEOSOURCESTREAM_H

#include <QObject>
#include <QImage>
#include <QString>

#define VideoSourceStream_iid "io.stateoftheart.handy3dscanner.VideoSource.VideoSourceStream"

class VideoSourceStream
{
public:
    VideoSourceStream(QString name, QString device_id, QString device_name)
        : m_name(name)
        , m_device_id(device_id)
        , m_device_name(device_name) {};
    virtual ~VideoSourceStream(){}

    virtual QString name() { return m_name; };
    virtual QString deviceId() { return m_device_id; };
    virtual QString deviceName() { return m_device_name; };

signals:
    // Signal on getting the new image
    virtual void newStreamImage(const QImage &image) = 0;

protected:
    QString m_name;
    QString m_device_id;
    QString m_device_name;
};

Q_DECLARE_INTERFACE(VideoSourceStream, VideoSourceStream_iid)

#endif // VIDEOSOURCESTREAM_H
