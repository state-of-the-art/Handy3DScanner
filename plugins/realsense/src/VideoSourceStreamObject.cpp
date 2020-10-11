#include "VideoSourceStreamObject.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(vsso, "RealSensePlugin::VideoSourceStreamObject")

VideoSourceStreamObject::VideoSourceStreamObject(QString name, QString device_id, QString device_name, int8_t type)
    : QObject()
    , VideoSourceStream(name, device_id, device_name)
    , rs2_stream_type(type)
{}
