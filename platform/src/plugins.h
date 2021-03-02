#ifndef PLUGINS_H
#define PLUGINS_H

#include <QObject>
#include <QMap>

#include "PluginInterface.h"

class Plugins
    : public QObject
{
    Q_OBJECT

public:
    inline static Plugins* I() { if( s_pInstance == nullptr ) s_pInstance = new Plugins(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    QList<QString> listPlugins();
    QList<QLatin1String> listInterfaces(const QString &name);

    // Plugin functions
    Q_INVOKABLE QList<QObject*> getInterfacePlugins(const QString &name);
    Q_INVOKABLE QObject* getPlugin(const QString &if_name, const QString &name);

    // Settings binding functions
    void settingActivePlugin(const QString &key, const QString &name);
    void settingActiveInterface(const QString &key, const QString &name, const QLatin1String &interface_id);

    bool activateInterface(const QString &name, const QLatin1String &interface_id);
    bool deactivateInterface(const QString &name, const QLatin1String &interface_id);

public slots:
    void settingChanged(const QString &key, const QVariant &value);

private:
    static Plugins *s_pInstance;
    explicit Plugins();
    ~Plugins() override;

    void refreshPluginsList();
    template<class T> bool addPlugin(T* obj, QObject *plugin);

    QMap<QString, QMap<QLatin1String, QObject*>> m_plugins; // plugin_name : interface_id : plugin_instance
    QMap<QLatin1String, QList<QObject*>> m_plugins_active; // interface_id : list of plugin_instance

    QMap<QString, QString> m_setting_plugin_active; // setting_key : plugin_name
    QMap<QString, QPair<QString, QLatin1String>> m_setting_plugin_interface_active; // setting_key : (plugin_name, interface_id)
};

#endif // PLUGINS_H
