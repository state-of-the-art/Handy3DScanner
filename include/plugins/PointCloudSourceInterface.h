#ifndef POINTCLOUDSOURCEINTERFACE_H
#define POINTCLOUDSOURCEINTERFACE_H

#include "PluginInterface.h"
#include "PointCloudData.h"

#define PointCloudSourceInterface_iid "io.stateoftheart.handy3dscanner.plugins.PointCloudSourceInterface"

class PointCloudSourceInterface : virtual public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(PointCloudSourceInterface_iid); }

    /**
     * @brief Receive current point cloud data for specified path
     * @param device_id - ID of the device
     * @return
     */
    virtual QSharedPointer<PointCloudData> getStreamPCData(const QString device_id) = 0;
};

Q_DECLARE_INTERFACE(PointCloudSourceInterface, PointCloudSourceInterface_iid)

#endif // POINTCLOUDSOURCEINTERFACE_H
