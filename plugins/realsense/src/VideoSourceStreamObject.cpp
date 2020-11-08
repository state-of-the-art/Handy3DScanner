#include "VideoSourceStreamObject.h"

#include <QDateTime>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(vsso, "RealSensePlugin::VideoSourceStreamObject")

VideoSourceStreamObject::VideoSourceStreamObject(QStringList path, QStringList description, int8_t type)
    : QObject()
    , VideoSourceStream(path, description)
    , rs2_stream_type(type)
{
    qCDebug(vsso) << "Created object" << type;
    connect(this, &VideoSourceStreamObject::newStreamImage, this, &VideoSourceStreamObject::updateLastFrameTime);
}

void VideoSourceStreamObject::updateLastFrameTime()
{
    m_last_frame_time = QDateTime::currentMSecsSinceEpoch();
}
