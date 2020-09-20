#ifndef REALSENSEPLUGIN_H
#define REALSENSEPLUGIN_H

#include <QObject>
#include "plugins/VideoSourceInterface.h"
#include "plugins/PointCloudSourceInterface.h"

#include "rsmanager.h"

class RealSensePlugin : public QObject, public VideoSourceInterface, public PointCloudSourceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.stateoftheart.handy3dscanner.plugins.RealSensePlugin")
    Q_INTERFACES(VideoSourceInterface PointCloudSourceInterface PluginInterface)

public:
    static RealSensePlugin *s_pInstance;
    ~RealSensePlugin() override {}

    // PluginInterface
    Q_INVOKABLE QString name() const override;
    QStringList requirements() const override;
    bool init() override; // Warning: executing multiple times for each interface
    bool deinit() override;
    bool configure() override;

    // VideoSourceInterface
    Q_INVOKABLE QStringList getAvailableStreams() const override;

    // PointCloudSourceInterface
    uint8_t* getPCData() const override;

signals:
    void appNotice(QString msg) override;
    void appWarning(QString msg) override;
    void appError(QString msg) override;

private:
    RSManager m_rsmanager; // Manager to listen on connected/disconnected cameras
};
#endif // REALSENSEPLUGIN_H
