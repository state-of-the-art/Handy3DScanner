#include "plugin.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(plugin, "RealSensePlugin")

QLatin1String RealSensePlugin::name() const
{
    return QLatin1String(plugin().categoryName());
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

bool RealSensePlugin::deinit()
{
    qCDebug(plugin) << "deinit()";
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

uint8_t *RealSensePlugin::getPCData() const
{
    qCDebug(plugin) << "getAvailableStreams()";
    return nullptr;
}
