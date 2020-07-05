#ifndef VIDEOSOURCEINTERFACE_H
#define VIDEOSOURCEINTERFACE_H

#include <QtPlugin>
#include <QStringList>
#include "PluginInterface.h"

#define VideoSourceInterface_iid "io.stateoftheart.handy3dscanner.plugins.VideoSourcePlugin"

class VideoSourceInterface : public PluginInterface
{
public:
    virtual QStringList getAvailableStreams() const = 0;
};

Q_DECLARE_INTERFACE(VideoSourceInterface, VideoSourceInterface_iid)

#endif // VIDEOSOURCEINTERFACE_H
