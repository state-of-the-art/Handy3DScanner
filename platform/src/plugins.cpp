#include "plugins.h"

#include <QLoggingCategory>
#include <QCoreApplication>

Q_LOGGING_CATEGORY(plugins, "Plugins")

#include <QStandardPaths>
#include <QDir>

#include <QPluginLoader>

#include "plugins/VideoSourceInterface.h"

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
            if( !addPlugin(qobject_cast<VideoSourceInterface *>(plugin)) )
                plugin_loader.unload();
        }
    }
}

bool Plugins::addPlugin(PluginInterface *plugin)
{
    if( plugin ) {
        if( !m_plugins.contains(plugin->name()) ) {
            m_plugins[plugin->name()] = plugin;
            qCDebug(plugins) << "  loaded plugin:" << plugin->name();
            return true;
        }
        qCWarning(plugins) << "  plugin already loaded, skipping:" << plugin->name();
    }
    return false;
}
