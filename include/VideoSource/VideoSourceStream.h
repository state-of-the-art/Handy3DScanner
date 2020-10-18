#ifndef VIDEOSOURCESTREAM_H
#define VIDEOSOURCESTREAM_H

#include <QObject>
#include <QImage>
#include <QString>

#define VideoSourceStream_iid "io.stateoftheart.handy3dscanner.VideoSource.VideoSourceStream"

class VideoSourceStream
{
public:
    VideoSourceStream(QStringList path, QStringList description)
        : m_path(path)
        , m_description(description) {};
    virtual ~VideoSourceStream(){}

    virtual QStringList path() { return m_path; };
    virtual QStringList description() { return m_description; };

signals:
    // Signal on getting the new image
    virtual void newStreamImage(const QImage &image) = 0;

protected:
    QStringList m_path;
    QStringList m_description;
};

Q_DECLARE_INTERFACE(VideoSourceStream, VideoSourceStream_iid)

#endif // VIDEOSOURCESTREAM_H
