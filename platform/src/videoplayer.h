#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QtQuick/qquickpainteditem.h>
#include <QtGui/qpainter.h>
#include <QtGui/qimage.h>
#include <QtQml/qqml.h>

class VideoPlayer
    : public QQuickPaintedItem
{
    Q_OBJECT

public:
    VideoPlayer();

    Q_INVOKABLE void setImage(QImage image);
    Q_INVOKABLE void reset();

    void paint(QPainter *painter);

    static void declareQML() {
        qmlRegisterType<VideoPlayer>("VideoPlayer", 1, 0, "VideoPlayer");
    }

private:
    QImage m_image;
};
#endif // VIDEOPLAYER_H
