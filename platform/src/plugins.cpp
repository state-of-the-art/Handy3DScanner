#include "plugins.h"
#include "settings.h"
#include "application.h"

#include <QLoggingCategory>
#include <QCoreApplication>

Q_LOGGING_CATEGORY(plugins, "Plugins")

#include <QStandardPaths>
#include <QDir>

#include <QPluginLoader>

#include "plugins/VideoSourceInterface.h"
#include "plugins/PointCloudSourceInterface.h"

Plugins* Plugins::s_pInstance = nullptr;

Plugins::Plugins()
    : QObject(nullptr)
{
    qCDebug(plugins, "Create object");
    refreshPluginsList();
}

Plugins::~Plugins()
{
    qCDebug(plugins, "Destroy object");
}

QList<QString> Plugins::listPlugins()
{
    return m_plugins.keys();
}

QList<QLatin1String> Plugins::listInterfaces(const QString &name)
{
    if( !m_plugins.contains(name) ) {
        qCWarning(plugins) << __func__ << "Not found plugin" << name;
        return QList<QLatin1String>();
    }
    return m_plugins[name].keys();
}

QList<QObject*> Plugins::getInterfacePlugins(const QString &if_name)
{
    QLatin1String iid = QLatin1String((if_name.toLatin1()));
    if( m_plugins_active.contains(iid) )
        return m_plugins_active[iid];
    else
        return QList<QObject*>();
}

QObject* Plugins::getPlugin(const QString &if_name, const QString &name)
{
    auto plugins = getInterfacePlugins(if_name);
    for( QObject* plugin : plugins ) {
        if( qobject_cast<PluginInterface *>(plugin)->name() == name )
            return plugin;
    }

    return nullptr;
}

void Plugins::settingActivePlugin(const QString &key, const QString &name)
{
    m_setting_plugin_active.insert(key, name);
}

void Plugins::settingActiveInterface(const QString &key, const QString &name, const QLatin1String &interface)
{
    m_setting_plugin_interface_active.insert(key, QPair<QString, QLatin1String>(name, interface));
}

bool Plugins::activateInterface(const QString &name, const QLatin1String &interface_id)
{
    // Don't activate interface if the plugin is not enabled
    if( !Settings::I()->val(m_setting_plugin_active.key(name)).toBool() )
        return false;
    QObject* plugin = m_plugins[name][interface_id];
    PluginInterface* plugin_if = qobject_cast<PluginInterface *>(plugin);
    if( !plugin_if ) {
        qCWarning(plugins) << __func__ << "Unable to locate plugin interface to activate" << name << plugin_if << plugin;
        return false;
    }
    plugin_if->init();
    if( !m_plugins_active[interface_id].contains(plugin) ) {
        m_plugins_active[interface_id].append(plugin);
        return true;
    }
    return false;
}

bool Plugins::deactivateInterface(const QString &name, const QLatin1String &interface_id)
{
    QObject* plugin = m_plugins[name][interface_id];
    PluginInterface* plugin_if = qobject_cast<PluginInterface *>(plugin);
    if( !plugin_if ) {
        qCWarning(plugins) << __func__ << "Unable to locate plugin interface to deactivate" << name << plugin_if << plugin;
        return false;
    }
    if( m_plugins_active[interface_id].contains(plugin) ) {
        m_plugins_active[interface_id].removeOne(plugin);
        // Deinit plugin if no interface active anymore
        auto it = m_plugins_active.begin();
        while( it != m_plugins_active.end() ) {
            if( it.value().contains(plugin) )
                return true;
            ++it;
        }
        plugin_if->deinit();
        return true;
    }
    return false;
}

void Plugins::settingChanged(const QString &key, const QVariant &value)
{
    if( m_setting_plugin_active.contains(key) ) {
        QString name = m_setting_plugin_active[key];
        qCDebug(plugins) << "Set plugin" << name << "active" << value.toBool();
        if( value.toBool() ) {
            auto it = m_setting_plugin_interface_active.begin();
            while( it != m_setting_plugin_interface_active.end() ) {
                if( it.value().first == name && Settings::I()->val(it.key()).toBool() )
                    activateInterface(name, it.value().second);
                ++it;
            }
        } else {
            auto it = m_setting_plugin_interface_active.begin();
            while( it != m_setting_plugin_interface_active.end() ) {
                if( it.value().first == name && Settings::I()->val(it.key()).toBool() )
                    deactivateInterface(name, it.value().second);
                ++it;
            }
        }
    } else if( m_setting_plugin_interface_active.contains(key) ) {
        QPair info = m_setting_plugin_interface_active[key];
        qCDebug(plugins) << "Set plugin" << info.first << "interface" << info.second << "active" << value.toBool();
        if( value.toBool() )
            activateInterface(info.first, info.second);
        else
            deactivateInterface(info.first, info.second);
    }
}

void Plugins::refreshPluginsList()
{
    QStringList plugins_dirs = { QCoreApplication::applicationDirPath() };
    QStringList filters = { "libh3ds-plugin-*" };

    for( const QString &path : plugins_dirs ) {
        qCDebug(plugins) << "Listing plugins from directory:" << QCoreApplication::applicationDirPath();
        QDir dir = QDir(path);
        dir.setNameFilters(filters);
        QStringList libs = dir.entryList(QDir::Files);
        for( const QString &lib_name : libs ) {
            QPluginLoader plugin_loader(dir.absoluteFilePath(lib_name));
            QObject *plugin = plugin_loader.instance();
            if( !plugin ) {
                qCWarning(plugins) << "  unable to load plugin:" << lib_name;
                continue;
            }

            // Connecting the message signals
            connect(plugin, SIGNAL(appNotice(QString)), Application::I(), SLOT(notice(QString)));
            connect(plugin, SIGNAL(appWarning(QString)), Application::I(), SLOT(warning(QString)));
            connect(plugin, SIGNAL(appError(QString)), Application::I(), SLOT(error(QString)));

            qCDebug(plugins) << "  loading plugin:" << lib_name;
            bool loaded = addPlugin<VideoSourceInterface>(qobject_cast<VideoSourceInterface *>(plugin), plugin);
            loaded = addPlugin<PointCloudSourceInterface>(qobject_cast<PointCloudSourceInterface *>(plugin), plugin) || loaded;

            if( !loaded ) {
                plugin_loader.unload();
                qCWarning(plugins) << "  no supported interfaces found for plugin:" << lib_name;
            }
        }
    }


    auto it = m_plugins.begin();
    while( it != m_plugins.end() ) {
        auto it2 = it.value().begin();
        while( it2 != it.value().end() ) {
            qCDebug(plugins) << "Loaded plugin" << it.key() << "for interface" << it2.key() << ":" << it2.value();
            ++it2;
        }
        ++it;
    }
}

template<class T>
bool Plugins::addPlugin(T *obj, QObject *plugin)
{
    static_assert(std::is_base_of<PluginInterface, T>::value, "Unable to add non-PluginInterface object as plugin");

    if( !obj )
        return false;

    if( m_plugins.contains(obj->name()) && m_plugins[obj->name()].contains(obj->type()) ) {
        qCWarning(plugins, "  plugin already loaded, skipping: %s::%s", obj->name().toUtf8().data(), obj->type().data());
        return false;
    }

    m_plugins[obj->name()][obj->type()] = plugin;
    qCDebug(plugins, "  loaded plugin: %s::%s", obj->name().toUtf8().data(), obj->type().data());
    return true;
}
