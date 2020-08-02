#ifndef REALSENSEPLUGIN_H
#define REALSENSEPLUGIN_H

#include <QObject>
#include "plugins/VideoSourceInterface.h"
#include "plugins/PointCloudSourceInterface.h"

class RealSensePlugin : public QObject, public VideoSourceInterface, public PointCloudSourceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.handy3dscanner.plugins.RealSensePlugin")
    Q_INTERFACES(VideoSourceInterface PointCloudSourceInterface)

public:
    ~RealSensePlugin() override {}

    // PluginInterface
    QLatin1String name() const override;
    QStringList requirements() const override;
    bool init() override; // Warning: executing multiple times for each interface
    bool deinit() override;
    bool configure() override;

    // VideoSourceInterface
    QStringList getAvailableStreams() const override;

    // PointCloudSourceInterface
    uint8_t* getPCData() const override;
};
#endif // REALSENSEPLUGIN_H
