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
     * @brief Return plugin type
     */
    static QLatin1String type() { return QLatin1String(PluginInterface_iid); }

    /**
     * @brief Plugin identify name
     */
    virtual QLatin1String name() const = 0;

    /**
     * @brief List of the plugins required by the plugin
     */
    virtual QStringList requirements() const = 0;

    /**
     * @brief Executed during plugin activation
     * Warning: will be executed for each interface
     */
    virtual bool init() = 0;

    /**
     * @brief Executed during plugin deactivation
     */
    virtual bool deinit() = 0;

    /**
     * @brief Executed when all the available plugins are initialized
     */
    virtual bool configure() = 0;

    /**
     * @brief Shows the plugin was initialized or not
     */
    bool isInitialized() { return m_initialized; }

signals:
    virtual void appNotice(QString msg) = 0;
    virtual void appWarning(QString msg) = 0;
    virtual void appError(QString msg) = 0;

protected:
    /**
     * @brief Used by plugin to set the init state
     * @param value
     */
    void setInitialized(bool value) { m_initialized = value; }

private:
    bool m_initialized = false;
};

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
