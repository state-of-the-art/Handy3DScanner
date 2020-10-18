#include "VideoSourceStreamObject.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(vsso, "RealSensePlugin::VideoSourceStreamObject")

VideoSourceStreamObject::VideoSourceStreamObject(QStringList path, QStringList description, int8_t type)
    : QObject()
    , VideoSourceStream(path, description)
    , rs2_stream_type(type)
{}
