#ifndef REALSENSEPLUGIN_H
#define REALSENSEPLUGIN_H

#include <QObject>
#include "plugins/VideoSourceInterface.h"

class RealSensePlugin : public QObject, public VideoSourceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID VideoSourceInterface_iid)
    Q_INTERFACES(VideoSourceInterface)

public:
    ~RealSensePlugin() override {}
    QString name() const override;
    QStringList requirements() const override;
    bool init() override;
    bool configure() override;
    QStringList getAvailableStreams() const override;
};
#endif // REALSENSEPLUGIN_H
