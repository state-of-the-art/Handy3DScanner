#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QStringList>

/**
 * Basic interface for plugins
 */
class PluginInterface
{
public:
    virtual ~PluginInterface() {};

    /**
     * Return plugin type
     */
    static QLatin1String type() { return QLatin1String("io.stateoftheart.handy3dscanner.Plugin"); }

    /**
     * Plugin identify name
     */
    virtual QLatin1String name() const = 0;

    /**
     * List of the plugins required by the plugin
     */
    virtual QStringList requirements() const = 0;

    /**
     * Executed during plugins registration
     */
    virtual bool init() = 0;

    /**
     * Executed when all the available plugins are initialized
     */
    virtual bool configure() = 0;
};

#endif // PLUGININTERFACE_H
