#include "videoplayer.h"

#include <QDateTime>
#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(videoplayer, "VideoPlayer")

VideoPlayer::VideoPlayer()
    : QQuickPaintedItem()
    , m_stream(nullptr)
    , m_streaming_timer(new QTimer(this))
    , m_is_streaming(false)
{
    // Timer to check that the active stream is ok
    m_streaming_timer->setInterval(1000);
    connect(m_streaming_timer, &QTimer::timeout, this, &VideoPlayer::checkStreaming);
}

VideoPlayer::~VideoPlayer()
{
    m_streaming_timer->stop();
}

void VideoPlayer::setStream(QObject *plugin, QStringList stream_path)
{
    VideoSourceInterface *plugin_if = qobject_cast<VideoSourceInterface*>(plugin);
    if( !plugin_if ) {
        qCWarning(videoplayer) << __func__ << "Incompatible plugin provided:" << plugin;
        return;
    }

    QObject *stream = plugin_if->getVideoStream(stream_path);
    if( !stream ) {
        qCWarning(videoplayer) << __func__ << "Null stream given by" << plugin << "for" << stream_path;
        return;
    }

    VideoSourceStream *stream_if = qobject_cast<VideoSourceStream*>(stream);
    if( !stream_if ) {
        qCWarning(videoplayer) << __func__ << "Unable find VideoSourceStream interface in" << stream;
        return;
    }

    if( m_stream && qobject_cast<VideoSourceStream*>(m_stream)->path() == stream_if->path() ) {
        qCDebug(videoplayer) << __func__ << "Do not reconnect the same stream";
        return;
    }

    if( m_stream ) {
        qCDebug(videoplayer) << __func__ << "Disconnecting the previous stream";
        disconnect(m_stream, SIGNAL(newStreamImage(QImage)), this, SLOT(setImage(QImage)));
    }

    m_stream = stream;
    emit currentStreamChanged();
    m_streaming_timer->start();

    qCDebug(videoplayer) << __func__ << "Connecting the new stream" << plugin << stream_path << m_stream;
    connect(m_stream, SIGNAL(newStreamImage(QImage)), this, SLOT(setImage(QImage)), Qt::QueuedConnection);
}

void VideoPlayer::paint(QPainter *painter) {
    QRectF bounding_rect = boundingRect();
    QImage scaled = this->m_image.scaledToHeight(bounding_rect.height());
    QPointF center = bounding_rect.center() - scaled.rect().center();

    if(center.x() < 0)
        center.setX(0);
    if(center.y() < 0)
        center.setY(0);

    painter->drawImage(center, scaled);
}

void VideoPlayer::setImage(const QImage image) {
    this->m_image = image;
    this->update();
}

void VideoPlayer::checkStreaming()
{
    if( ! m_stream ) {
        m_is_streaming = false;
        emit isStreamingChanged();
        m_streaming_timer->stop();
        return;
    }

    if( qobject_cast<VideoSourceStream*>(m_stream)->lastFrameTime() + 1000 > QDateTime::currentMSecsSinceEpoch() ) {
        if( !m_is_streaming ) {
            m_is_streaming = true;
            emit isStreamingChanged();
        }
    } else {
        if( m_is_streaming ) {
            m_is_streaming = false;
            emit isStreamingChanged();
        }
    }
}

void VideoPlayer::reset() {
    setImage(QImage());
}

QString VideoPlayer::currentStream()
{
    if( !m_stream )
        return QString();

    QStringList path = qobject_cast<VideoSourceStream*>(m_stream)->path();
    return path.join("->");
}
