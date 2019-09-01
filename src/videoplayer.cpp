#include "videoplayer.h"
#include <QDebug>

VideoPlayer::VideoPlayer()
    : QQuickPaintedItem() {}

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

void VideoPlayer::setImage(QImage image){
    this->m_image = image;
    this->update();
}

void VideoPlayer::reset() {
    setImage(QImage());
}
