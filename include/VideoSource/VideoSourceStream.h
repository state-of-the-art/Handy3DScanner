#ifndef VIDEOSOURCESTREAM_H
#define VIDEOSOURCESTREAM_H

#include <QObject>
#include <QImage>
#include <QString>

#define VideoSourceStream_iid "io.stateoftheart.handy3dscanner.VideoSource.VideoSourceStream"

class VideoSourceStream
{
public:
    VideoSourceStream(QString plugin_name, QStringList path, QStringList description)
        : m_plugin_name(plugin_name)
        , m_path(path)
        , m_description(description)
        , m_capture(false)
        , m_last_frame_time(0) {};
    virtual ~VideoSourceStream() {}

    virtual QString pluginName() { return m_plugin_name; };
    virtual QStringList path() { return m_path; };
    virtual QStringList description() { return m_description; };
    virtual qint64 lastFrameTime() { return m_last_frame_time; };

    virtual bool isCapture() { return m_capture; };
    virtual void setCapture(const bool value) { if( m_capture != value ) { m_capture = value; emit captureChanged(value); } };

signals:
    // Signal on getting the new image
    virtual void newStreamImage(const QImage &image) = 0;
    // Signal on change capture flag
    virtual void captureChanged(const bool value) = 0;

    virtual void fps(qreal) = 0; // Frames per second
    virtual void fwt(qreal) = 0; // Frame wait time
    virtual void fpt(qreal) = 0; // Frame processing time

protected:
    QString m_plugin_name;
    QStringList m_path;
    QStringList m_description;
    bool m_capture;

    qint64 m_last_frame_time;
};

Q_DECLARE_INTERFACE(VideoSourceStream, VideoSourceStream_iid)

#endif // VIDEOSOURCESTREAM_H
