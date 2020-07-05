#include "plugin.h"

#include <QDebug>

QStringList RealSensePlugin::requirements() const
{
    qDebug() << "RealSensePlugin::requirements()";
    return QStringList();
}

bool RealSensePlugin::init()
{
    qDebug() << "RealSensePlugin::init()";
    return true;
}

bool RealSensePlugin::configure()
{
    qDebug() << "RealSensePlugin::configure()";
    return true;
}

QStringList RealSensePlugin::getAvailableStreams() const
{
    qDebug() << "RealSensePlugin::getAvailableStreams()";
    return QStringList();
}
