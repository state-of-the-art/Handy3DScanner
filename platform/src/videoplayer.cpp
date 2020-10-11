#include "videoplayer.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(videoplayer, "VideoPlayer")

VideoPlayer::VideoPlayer()
    : QQuickPaintedItem()
    , m_stream(nullptr)
{}

void VideoPlayer::setStream(QObject *plugin, QString stream_path)
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

    if( m_stream && qobject_cast<VideoSourceStream*>(m_stream)->name() == stream_if->name() ) {
        qCDebug(videoplayer) << __func__ << "Do not reconnect the same stream";
        return;
    }

    if( m_stream ) {
        qCDebug(videoplayer) << __func__ << "Disconnecting the previous stream";
        disconnect(this, SLOT(setImage(QImage)));
    }

    m_stream = stream;

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

void VideoPlayer::reset() {
    setImage(QImage());
}
