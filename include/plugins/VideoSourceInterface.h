#ifndef VIDEOSOURCEINTERFACE_H
#define VIDEOSOURCEINTERFACE_H

#include <QStringList>
#include "PluginInterface.h"

#define VideoSourceInterface_iid "io.stateoftheart.handy3dscanner.plugins.VideoSourceInterface"

class VideoSourceInterface : public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(VideoSourceInterface_iid); }

    virtual QStringList getAvailableStreams() const = 0;
};

Q_DECLARE_INTERFACE(VideoSourceInterface, VideoSourceInterface_iid)

#endif // VIDEOSOURCEINTERFACE_H
