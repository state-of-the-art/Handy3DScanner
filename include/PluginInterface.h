#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QStringList>

#define PluginInterface_iid "io.stateoftheart.handy3dscanner.PluginInterface"

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
    static QLatin1String type() { return QLatin1String(PluginInterface_iid); }

    /**
     * Plugin identify name
     */
    virtual QLatin1String name() const = 0;

    /**
     * List of the plugins required by the plugin
     */
    virtual QStringList requirements() const = 0;

    /**
     * Executed during plugin activation
     * Warning: will be executed for each interface
     */
    virtual bool init() = 0;

    /**
     * Executed during plugin deactivation
     */
    virtual bool deinit() = 0;

    /**
     * Executed when all the available plugins are initialized
     */
    virtual bool configure() = 0;
};

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
