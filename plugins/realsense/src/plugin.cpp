#include "plugin.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(plugin, "RealSensePlugin")

QString RealSensePlugin::name() const
{
    return plugin().categoryName();
}

QStringList RealSensePlugin::requirements() const
{
    qCDebug(plugin) << "requirements()";
    return QStringList();
}

bool RealSensePlugin::init()
{
    qCDebug(plugin) << "init()";
    return true;
}

bool RealSensePlugin::configure()
{
    qCDebug(plugin) << "configure()";
    return true;
}

QStringList RealSensePlugin::getAvailableStreams() const
{
    qCDebug(plugin) << "getAvailableStreams()";
    return QStringList();
}
