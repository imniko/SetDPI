#include "DpiHelper.h"
#include <memory>
#include <cassert>



bool DpiHelper::GetPathsAndModes(std::vector<DISPLAYCONFIG_PATH_INFO>& pathsV, std::vector<DISPLAYCONFIG_MODE_INFO>& modesV, int flags)
{
    UINT32 numPaths = 0, numModes = 0;
    auto status = GetDisplayConfigBufferSizes(flags, &numPaths, &numModes);
    if (ERROR_SUCCESS != status)
    {
        return false;
    }

    std::unique_ptr<DISPLAYCONFIG_PATH_INFO[]> paths(new DISPLAYCONFIG_PATH_INFO[numPaths]);
    std::unique_ptr<DISPLAYCONFIG_MODE_INFO[]> modes(new DISPLAYCONFIG_MODE_INFO[numModes]);
    status = QueryDisplayConfig(flags, &numPaths, paths.get(), &numModes, modes.get(), nullptr);
    if (ERROR_SUCCESS != status)
    {
        return false;
    }

    for (unsigned int i = 0; i < numPaths; i++)
    {
        pathsV.push_back(paths[i]);
    }

    for (unsigned int i = 0; i < numModes; i++)
    {
        modesV.push_back(modes[i]);
    }

    return true;
}


DpiHelper::DpiHelper()
{
}


DpiHelper::~DpiHelper()
{
}


DpiHelper::DPIScalingInfo DpiHelper::GetDPIScalingInfo(LUID adapterID, UINT32 sourceID)
{
    DPIScalingInfo dpiInfo = {};

    DpiHelper::DISPLAYCONFIG_SOURCE_DPI_SCALE_GET requestPacket = {};
    requestPacket.header.type = (DISPLAYCONFIG_DEVICE_INFO_TYPE)DpiHelper::DISPLAYCONFIG_DEVICE_INFO_TYPE_CUSTOM::DISPLAYCONFIG_DEVICE_INFO_GET_DPI_SCALE;
    requestPacket.header.size = sizeof(requestPacket);
    assert(0x20 == sizeof(requestPacket));//if this fails => OS has changed somthing, and our reverse enginnering knowledge about the API is outdated
    requestPacket.header.adapterId = adapterID;
    requestPacket.header.id = sourceID;

    auto res = ::DisplayConfigGetDeviceInfo(&requestPacket.header);
    if (ERROR_SUCCESS == res)
    {//success
        if (requestPacket.curScaleRel < requestPacket.minScaleRel)
        {
            requestPacket.curScaleRel = requestPacket.minScaleRel;
        }
        else if (requestPacket.curScaleRel > requestPacket.maxScaleRel)
        {
            requestPacket.curScaleRel = requestPacket.maxScaleRel;
        }

        std::int32_t minAbs = abs((int)requestPacket.minScaleRel);
        if (DpiHelper::CountOf(DpiVals) >= (size_t)(minAbs + requestPacket.maxScaleRel + 1))
        {//all ok
            dpiInfo.current = DpiVals[minAbs + requestPacket.curScaleRel];
            dpiInfo.recommended = DpiVals[minAbs];
            dpiInfo.maximum = DpiVals[minAbs + requestPacket.maxScaleRel];
            dpiInfo.bInitDone = true;
        }
        else
        {
            //Error! Probably DpiVals array is outdated
            return dpiInfo;
        }
    }
    else
    {
        //DisplayConfigGetDeviceInfo() failed
        return dpiInfo;
    }

    return dpiInfo;
}


bool DpiHelper::SetDPIScaling(LUID adapterID, UINT32 sourceID, UINT32 dpiPercentToSet)
{
    DPIScalingInfo dPIScalingInfo = GetDPIScalingInfo(adapterID, sourceID);

    if (dpiPercentToSet == dPIScalingInfo.current)
    {
        return true;
    }

    if (dpiPercentToSet < dPIScalingInfo.mininum)
    {
        dpiPercentToSet = dPIScalingInfo.mininum;
    }
    else if (dpiPercentToSet > dPIScalingInfo.maximum)
    {
        dpiPercentToSet = dPIScalingInfo.maximum;
    }

    int idx1 = -1, idx2 = -1;

    int i = 0;
    for (const auto& val : DpiVals)
    {
        if (val == dpiPercentToSet)
        {
            idx1 = i;
        }

        if (val == dPIScalingInfo.recommended)
        {
            idx2 = i;
        }
        i++;
    }

    if ((idx1 == -1) || (idx2 == -1))
    {
        //Error cannot find dpi value
        return false;
    }

    int dpiRelativeVal = idx1 - idx2;

    DpiHelper::DISPLAYCONFIG_SOURCE_DPI_SCALE_SET setPacket = {};
    setPacket.header.adapterId = adapterID;
    setPacket.header.id = sourceID;
    setPacket.header.size = sizeof(setPacket);
    assert(0x18 == sizeof(setPacket));//if this fails => OS has changed somthing, and our reverse enginnering knowledge about the API is outdated
    setPacket.header.type = (DISPLAYCONFIG_DEVICE_INFO_TYPE)DpiHelper::DISPLAYCONFIG_DEVICE_INFO_TYPE_CUSTOM::DISPLAYCONFIG_DEVICE_INFO_SET_DPI_SCALE;
    setPacket.scaleRel = (UINT32)dpiRelativeVal;

    auto res = ::DisplayConfigSetDeviceInfo(&setPacket.header);
    if (ERROR_SUCCESS == res)
    {
        return true;
    }
    else
    {
        return false;
    }
    return true;
}
