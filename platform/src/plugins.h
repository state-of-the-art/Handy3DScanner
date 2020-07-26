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

private:
    static Plugins *s_pInstance;
    explicit Plugins();
    ~Plugins() override;

    void refreshPluginsList();
    template<class T> bool addPlugin(T* plugin);

    QMap<QLatin1String, QMap<QLatin1String, PluginInterface*>> m_plugins; // name : interface_id : plugin instance
    QMap<QLatin1String, QList<PluginInterface*>> m_plugins_active; // interface_id : list of plugins
};

#endif // PLUGINS_H
