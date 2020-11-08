#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QtQuick/qquickpainteditem.h>
#include <QtGui/qpainter.h>
#include <QtGui/qimage.h>
#include <QtQml/qqml.h>

#include "plugins/VideoSourceInterface.h"

class QTimer;

class VideoPlayer
    : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString currentStream READ currentStream NOTIFY currentStreamChanged)
    Q_PROPERTY(bool isStreaming READ isStreaming NOTIFY isStreamingChanged)

public:
    VideoPlayer();
    ~VideoPlayer();

    Q_INVOKABLE void setStream(/*VideoSourceInterface*/QObject* plugin, QStringList stream_path);
    Q_INVOKABLE void reset();

    QString currentStream();
    bool isStreaming() { return m_is_streaming; };

    void paint(QPainter *painter) override;

    static void declareQML() {
        qmlRegisterType<VideoPlayer>("VideoPlayer", 1, 0, "VideoPlayer");
    }

signals:
    void currentStreamChanged();
    void isStreamingChanged();

public slots:
    void setImage(const QImage image);

private:
    QObject *m_stream;
    QImage m_image;

    QTimer *m_streaming_timer;
    bool m_is_streaming;

private slots:
    void checkStreaming();
};
#endif // VIDEOPLAYER_H
