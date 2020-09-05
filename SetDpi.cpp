#include <iostream>
#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <cstringt.h>
#include "DpiHelper.h"
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

void SetDpiScaling(int percentScaleToSet)
{
    int recommendedDpiScale = GetRecommendedDPIScaling();

    if (recommendedDpiScale > 0)
    {
        int index = 0, recIndex = 0, setIndex = 0;
        for (const auto& scale : DpiVals)
        {
            if (recommendedDpiScale == scale)
            {
                recIndex = index;
            }
            if (percentScaleToSet == scale)
            {
                setIndex = index;
            }
            index++;
        }

        int relativeIndex = setIndex - recIndex;
        SystemParametersInfo(SPI_SETLOGICALDPIOVERRIDE, relativeIndex, (LPVOID)0, 1);
    }
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
std::map<int, DisplayData> m_displayDataCache;


void GetDisplayData()
{
    m_displayDataCache.clear();
    std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
    std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
    int flags = QDC_ONLY_ACTIVE_PATHS;
    if (false == DpiHelper::GetPathsAndModes(pathsV, modesV, flags))
    {
        cout << "DpiHelper::GetPathsAndModes() failed\n";
    }
    else
    {
        cout << "DpiHelper::GetPathsAndModes() successful\n";
    }
    int iIndex = 0;
    for (const auto& path : pathsV)
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
            std::wstring nameString = std::to_wstring(iIndex) + std::wstring(L". ") + deviceName.monitorFriendlyDeviceName;
            if (DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL == deviceName.outputTechnology)
            {
                nameString += L"(internal display)";
            }
            DisplayData dd = {};
            dd.m_adapterId = adapterLUID;
            dd.m_sourceID = sourceID;
            dd.m_targetID = targetID;

            m_displayDataCache[iIndex] = dd;
            iIndex++;


        }
    }
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
int main(int argc, char* argv[])
{
    int n = 0, dpiToSet = 0;
    if (argc == 2)
    {
        dpiToSet = atoi(argv[1]);
        if (!DPIFound(dpiToSet))
        {
            cout << "Invalid DPI scale value: " << dpiToSet;
            return 0;
        }
        SetDpiScaling(dpiToSet);
    }
    else if (argc == 3)
    {
        GetDisplayData();
        int displayIndex = atoi(argv[1]);
        int dpiToSet = atoi(argv[2]);
        bool notFound = true;
        if (!DPIFound(dpiToSet))
        {
            cout << "Invalid DPI scale value: " << dpiToSet;
            return 0;
        }
        if (displayIndex <= m_displayDataCache.size() && displayIndex > 0)
        {
            auto res = DpiHelper::SetDPIScaling(m_displayDataCache[displayIndex - 1].m_adapterId, m_displayDataCache[displayIndex - 1].m_sourceID, dpiToSet);
            if (false == res)
            {
                cout << "DpiHelper::SetDPIScaling() failed";
                return 0;
            }
        }
        else
        {
            cout << "Invalid Monitor ID: " << displayIndex;
            return 0;
        }
    }
    else
    {
        cout << "Error: Provide a dpi scale (e.g. 150) as command line argument";
    }

    return 0;
}
