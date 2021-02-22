#include "videoplayer.h"
#include "plugins.h"

#include <QDateTime>
#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(videoplayer, "VideoPlayer")

VideoPlayer::VideoPlayer()
    : QQuickPaintedItem()
    , m_stream(nullptr)
    , m_streaming_timer(new QTimer(this))
    , m_is_streaming(false)
    , m_stream_fps(0.0)
    , m_stream_fwt(0.0)
    , m_stream_fpt(0.0)
{
    // Timer to check that the active stream is ok
    m_streaming_timer->setInterval(1000);
    connect(m_streaming_timer, &QTimer::timeout, this, &VideoPlayer::checkStreaming);
}

VideoPlayer::~VideoPlayer()
{
    m_streaming_timer->stop();
}

void VideoPlayer::setStream(QStringList stream_path)
{
    qCDebug(videoplayer) << __func__ << "Set current stream:" << stream_path;
    QObject *plugin = Plugins::I()->getPlugin(VideoSourceInterface_iid, stream_path.first());
    if( !plugin ) {
        qCWarning(videoplayer) << __func__ << "Not found the required plugin:" << stream_path.first();
        return;
    }

    VideoSourceInterface *plugin_if = qobject_cast<VideoSourceInterface*>(plugin);
    if( !plugin_if ) {
        qCWarning(videoplayer) << __func__ << "Incompatible plugin provided:" << plugin;
        return;
    }

    stream_path.removeFirst();
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
    connect(m_stream, SIGNAL(fps(qreal)), this, SLOT(setStreamFPS(const qreal)), Qt::QueuedConnection);
    connect(m_stream, SIGNAL(fwt(qreal)), this, SLOT(setStreamFWT(const qreal)), Qt::QueuedConnection);
    connect(m_stream, SIGNAL(fpt(qreal)), this, SLOT(setStreamFPT(const qreal)), Qt::QueuedConnection);
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
