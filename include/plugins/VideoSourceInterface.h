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

    /**
     * @brief The tree of available streams
     * @details Returns a tree with the next parameters:
     * \code
     *   {
     *     "id": {                                     // Path identifier
     *       "name": QString,                          // Readable name to use instead of id (optional)
     *       "description": QString,                   // Additional info (optional)
     *       "supported": boolean,                     // Plugin can process this stream (default true) (optional)
     *       "childrens": { "id": {"name", ...}, ... } // Leafs of the tree - the same format here(optional)
     *     },
     *     ...
     *   }
     * \endcode
     * @return QMap<QString,QVariantMap>
     */
    Q_INVOKABLE virtual QVariantMap /*<QString,QVariantMap>*/ getAvailableStreams() const = 0;
    // In fact returns {VideoSourceStream}-based impl since no way to dynamic_cast back to QObject from plugin
    Q_INVOKABLE virtual /*VideoSourceStream*/QObject* getVideoStream(const QStringList path) = 0;
};

Q_DECLARE_INTERFACE(VideoSourceInterface, VideoSourceInterface_iid)

#endif // VIDEOSOURCEINTERFACE_H
