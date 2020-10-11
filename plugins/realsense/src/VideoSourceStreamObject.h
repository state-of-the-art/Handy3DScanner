#ifndef VIDEOSOURCESTREAMOBJECT_H
#define VIDEOSOURCESTREAMOBJECT_H

#include "VideoSource/VideoSourceStream.h"

class VideoSourceStreamObject : public QObject, public VideoSourceStream
{
    Q_OBJECT
    Q_INTERFACES(VideoSourceStream)

public:
    VideoSourceStreamObject(QString name, QString device_id, QString device_name, int8_t type);
    int8_t rs2_stream_type;

signals:
    void newStreamImage(const QImage &image);
};
#endif // VIDEOSOURCESTREAMOBJECT_H
