#ifndef VIDEOSOURCEINTERFACE_H
#define VIDEOSOURCEINTERFACE_H

#include <QStringList>
#include "PluginInterface.h"
#include "VideoSource/VideoSourceStream.h"

#define VideoSourceInterface_iid "io.stateoftheart.handy3dscanner.plugins.VideoSourceInterface"

class VideoSourceInterface : virtual public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(VideoSourceInterface_iid); }

    Q_INVOKABLE virtual QVariantMap /*<QString,QString>*/ getAvailableStreams(QStringList path) const = 0;
    // In fact returns {VideoSourceStream}-based impl since no way to dynamic_cast back to QObject from plugin
    Q_INVOKABLE virtual /*VideoSourceStream*/QObject* getVideoStream(const QStringList path) = 0;
};

Q_DECLARE_INTERFACE(VideoSourceInterface, VideoSourceInterface_iid)

#endif // VIDEOSOURCEINTERFACE_H
