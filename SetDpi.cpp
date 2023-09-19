#include "DpiHelper.h"
#include <Windows.h>
#include <cstringt.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

/*Get default DPI scaling percentage.
The OS recommented value.
*/
int GetRecommendedDPIScaling()
{
    int dpi = 0;

    auto retval = SystemParametersInfo(SPI_GETLOGICALDPIOVERRIDE, 0, (LPVOID)&dpi, 1);
    if (retval != 0)
    {
        int currDPI = DpiVals[dpi * -1];
        return currDPI;
    }

    return -1;
}

//to store display info along with corresponding list item
struct DisplayData {
    LUID m_adapterId;
    int m_targetID;
    int m_sourceID;

    DisplayData()
    {
        m_adapterId = {};
        m_targetID = m_sourceID = -1;
    }
};

std::vector<DisplayData> GetDisplayData()
{
    std::vector<DisplayData> displayDataCache;
    std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
    std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
    int flags = QDC_ONLY_ACTIVE_PATHS;
    if (false == DpiHelper::GetPathsAndModes(pathsV, modesV, flags))
    {
        cout << "DpiHelper::GetPathsAndModes() failed\n";
    }
    displayDataCache.resize(pathsV.size());
    int idx = 0;
    for (const auto &path : pathsV)
    {
        //get display name
        auto adapterLUID = path.targetInfo.adapterId;
        auto targetID = path.targetInfo.id;
        auto sourceID = path.sourceInfo.id;

        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName;
        deviceName.header.size = sizeof(deviceName);
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.adapterId = adapterLUID;
        deviceName.header.id = targetID;
        if (ERROR_SUCCESS != DisplayConfigGetDeviceInfo(&deviceName.header))
        {
            cout << "DisplayConfigGetDeviceInfo() failed";
        }
        else
        {
            std::wstring nameString = std::to_wstring(idx) + std::wstring(L". ") + deviceName.monitorFriendlyDeviceName;
            if (DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL == deviceName.outputTechnology)
            {
                nameString += L"(internal display)";
            }
            DisplayData dd = {};
            dd.m_adapterId = adapterLUID;
            dd.m_sourceID = sourceID;
            dd.m_targetID = targetID;

            displayDataCache[idx] = dd;
        }
        idx++;
    }
    return displayDataCache;
}

bool DPIFound(int val)
{
    bool found = false;
    for (int i = 0; i < 12; i++)
    {
        if (val == DpiVals[i])
        {
            found = true;
            break;
        }
    }
    return found;
}
int main(int argc, char *argv[])
{
    auto dpiToSet = 0;
    auto displayIndex = 1;

    enum
    {
        RESOLUTION_SET,
        RESOLUTION_GET,
        RESOLUTION_VALUE,
    } resolutionMode = RESOLUTION_SET;

    if (argc <= 1)
    {
        cout << "1. argument: Resolution in percent, use \"get\" to print the current value instead and \"value\" to print without formatting\n";
        cout << "2. argument: Monitor index, leave empty to use primary monitor\n";
        return 0;
    }

    if (argc >= 2)
    {
        if (strcmp(argv[1], "get") == 0)
        {
            resolutionMode = RESOLUTION_GET;
        }
        else if (strcmp(argv[1], "value") == 0)
        {
            resolutionMode = RESOLUTION_VALUE;
        }
        else
        {
            dpiToSet = atoi(argv[1]);
        }
    }

    if (argc >= 3)
    {
        displayIndex = atoi(argv[2]);
    }

    auto displayDataCache = GetDisplayData();
    if (displayIndex < 1 || displayDataCache.size() < displayIndex)
    {
        if (DPIFound(displayIndex) && 1 <= dpiToSet && dpiToSet <= displayDataCache.size())
        {
            cout << "Please provide the scale as first and the index as second argument, program will continue for legacy purposes\n";
            auto t = dpiToSet;
            dpiToSet = displayIndex;
            displayIndex = t;
        }
        else
        {
            cout << "Invalid Monitor ID: " << displayIndex;
            return 0;
        }
    }

    displayIndex -= 1; // change from 1...X to 0...(X-1)

    auto currentResolution = DpiHelper::GetDPIScalingInfo(displayDataCache[displayIndex].m_adapterId, displayDataCache[displayIndex].m_sourceID);
    if (resolutionMode == RESOLUTION_GET)
    {
        cout << "Current Resolution: " << currentResolution.current;
        return 0;
    }
    if (resolutionMode == RESOLUTION_VALUE)
    {
        cout << currentResolution.current;
        return 0;
    }
    if (!DPIFound(dpiToSet))
    {
        cout << "Invalid DPI scale value: " << dpiToSet;
        return 0;
    }
    auto success = DpiHelper::SetDPIScaling(displayDataCache[displayIndex].m_adapterId, displayDataCache[displayIndex].m_sourceID, dpiToSet);
    if (success == false)
    {
        cout << "DpiHelper::SetDPIScaling() failed";
        return 0;
    }
    else
    {
        if (displayIndex == 0)
        {
            HKEY hKey;
            LPCWSTR sKeyPath;
            DWORD value = static_cast<DWORD>(int(dpiToSet * 0.96));
            int iResult;
            sKeyPath = L"Control Panel\\Desktop\\WindowMetrics\\";
            iResult = RegOpenKeyEx(HKEY_CURRENT_USER, sKeyPath, NULL, KEY_ALL_ACCESS, &hKey);
            iResult = RegSetValueEx(hKey, L"AppliedDPI", NULL, REG_DWORD, (const BYTE*)&value, sizeof(value));
            RegCloseKey(hKey);
            return 0;
        }
    }
}
