#ifndef POINTCLOUDSOURCEINTERFACE_H
#define POINTCLOUDSOURCEINTERFACE_H

#include "PluginInterface.h"

#define PointCloudSourceInterface_iid "io.stateoftheart.handy3dscanner.plugins.PointCloudSourceInterface"

class PointCloudSourceInterface : public PluginInterface
{
public:
    static QLatin1String type() { return QLatin1String(PointCloudSourceInterface_iid); }

    virtual uint8_t* getPCData() const = 0;
};

Q_DECLARE_INTERFACE(PointCloudSourceInterface, PointCloudSourceInterface_iid)

#endif // POINTCLOUDSOURCEINTERFACE_H
