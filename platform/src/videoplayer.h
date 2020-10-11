#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QtQuick/qquickpainteditem.h>
#include <QtGui/qpainter.h>
#include <QtGui/qimage.h>
#include <QtQml/qqml.h>

#include "plugins/VideoSourceInterface.h"

class VideoPlayer
    : public QQuickPaintedItem
{
    Q_OBJECT

public:
    VideoPlayer();

    Q_INVOKABLE void setStream(/*VideoSourceInterface*/QObject* plugin, QString stream_path);
    Q_INVOKABLE void reset();

    void paint(QPainter *painter) override;

    static void declareQML() {
        qmlRegisterType<VideoPlayer>("VideoPlayer", 1, 0, "VideoPlayer");
    }

private:
    QObject *m_stream;
    QImage m_image;

public slots:
    void setImage(const QImage image);
};
#endif // VIDEOPLAYER_H
