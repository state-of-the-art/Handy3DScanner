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
    Q_PROPERTY(qreal streamFPS READ getStreamFPS NOTIFY streamFPSChanged)
    Q_PROPERTY(qreal streamFWT READ getStreamFWT NOTIFY streamFWTChanged)
    Q_PROPERTY(qreal streamFPT READ getStreamFPT NOTIFY streamFPTChanged)

public:
    VideoPlayer();
    ~VideoPlayer();

    Q_INVOKABLE void setStream(QStringList stream_path);
    Q_INVOKABLE void reset();

    QString currentStream();
    bool isStreaming() { return m_is_streaming; };

    void paint(QPainter *painter) override;

    static void declareQML() {
        qmlRegisterType<VideoPlayer>("VideoPlayer", 1, 0, "VideoPlayer");
    }

    qreal getStreamFPS() const { return m_stream_fps; }
    qreal getStreamFWT() const { return m_stream_fwt; }
    qreal getStreamFPT() const { return m_stream_fpt; }

signals:
    void currentStreamChanged();
    void isStreamingChanged();
    void streamFPSChanged();
    void streamFWTChanged();
    void streamFPTChanged();

public slots:
    void setImage(const QImage image);
    void setStreamFPS(const qreal fps) { m_stream_fps = fps; emit streamFPSChanged(); }
    void setStreamFWT(const qreal fwt) { m_stream_fwt = fwt; emit streamFWTChanged(); }
    void setStreamFPT(const qreal fpt) { m_stream_fpt = fpt; emit streamFPTChanged(); }

private:
    QObject *m_stream;
    QImage m_image;

    QTimer *m_streaming_timer;
    bool m_is_streaming;

    qreal m_stream_fps;
    qreal m_stream_fwt;
    qreal m_stream_fpt;

private slots:
    void checkStreaming();
};
#endif // VIDEOPLAYER_H
