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
    bool addPlugin(PluginInterface* plugin);

    QMap<QString, PluginInterface*> m_plugins;
};

#endif // PLUGINS_H
