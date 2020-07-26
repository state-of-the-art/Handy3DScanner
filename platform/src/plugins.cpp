#include "plugins.h"

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

void Plugins::refreshPluginsList()
{
    QStringList plugins_dirs = { QCoreApplication::applicationDirPath() };
    QStringList filters = { "h3ds-plugin-*" };

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

            qCDebug(plugins) << "  loading plugin:" << lib_name;
            bool loaded = addPlugin<VideoSourceInterface>(qobject_cast<VideoSourceInterface *>(plugin));
            loaded = addPlugin<PointCloudSourceInterface>(qobject_cast<PointCloudSourceInterface *>(plugin)) || loaded;

            if( !loaded ) {
                plugin_loader.unload();
                qCWarning(plugins) << "  no supported interfaces found for plugin:" << lib_name;
            }
        }
    }
}

template<class T>
bool Plugins::addPlugin(T *plugin)
{
    static_assert(std::is_base_of<PluginInterface, T>::value, "Unable to add non-PluginInterface object as plugin");

    if( !plugin )
        return false;

    if( m_plugins.contains(plugin->name()) && m_plugins[plugin->name()].contains(plugin->type()) ) {
        qCWarning(plugins, "  plugin already loaded, skipping: %s::%s", plugin->name().data(), plugin->type().data());
        return false;
    }

    m_plugins[plugin->name()][plugin->type()] = plugin;
    qCDebug(plugins, "  loaded plugin: %s::%s", plugin->name().data(), plugin->type().data());
    return true;
}
