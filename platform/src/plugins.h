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

    QList<QLatin1String> listPlugins();
    QList<QLatin1String> listInterfaces(const QLatin1String &name);

    void settingActivePlugin(const QString &key, const QLatin1String &name);
    void settingActiveInterface(const QString &key, const QLatin1String &name, const QLatin1String &interface);

    bool activateInterface(const QLatin1String &name, const QLatin1String &interface);
    bool deactivateInterface(const QLatin1String &name, const QLatin1String &interface);

public slots:
    void settingChanged(const QString &key, const QVariant &value);

private:
    static Plugins *s_pInstance;
    explicit Plugins();
    ~Plugins() override;

    void refreshPluginsList();
    template<class T> bool addPlugin(T* obj, PluginInterface **plugin);

    QMap<QLatin1String, QMap<QLatin1String, PluginInterface*>> m_plugins; // plugin_name : interface_id : plugin_instance
    QMap<QLatin1String, QList<PluginInterface*>> m_plugins_active; // interface_id : list of plugins

    QMap<QString, QLatin1String> m_setting_plugin_active; // setting_key : plugin_name
    QMap<QString, QPair<QLatin1String, QLatin1String>> m_setting_plugin_interface_active; // setting_key : (plugin_name, interface_id)
};

#endif // PLUGINS_H
