#ifndef VIDEOSOURCESTREAMOBJECT_H
#define VIDEOSOURCESTREAMOBJECT_H

#include "VideoSource/VideoSourceStream.h"

class VideoSourceStreamObject : public QObject, public VideoSourceStream
{
    Q_OBJECT
    Q_INTERFACES(VideoSourceStream)

public:
    VideoSourceStreamObject(QStringList path, QStringList description, int8_t type);
    int8_t rs2_stream_type;

signals:
    void newStreamImage(const QImage &image);
    void captureChanged(const bool value);

private slots:
    void updateLastFrameTime();
};
#endif // VIDEOSOURCESTREAMOBJECT_H
