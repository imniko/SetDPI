#pragma once
#include <Windows.h>
#include <vector>


/*
* OS reports DPI scaling values in relative terms, and not absolute terms.
* eg. if current DPI value is 250%, and recommended value is 200%, then
* OS will give us integer 2 for DPI scaling value (starting from recommended
* DPI scaling move 2 steps to the right in this list).
* values observed (and extrapolated) from system settings app (immersive control panel).
*/
static const UINT32 DpiVals[] = { 100,125,150,175,200,225,250,300,350, 400, 450, 500 };

class DpiHelper
{
public:
    template<class T, size_t sz>
    static size_t CountOf(const T (&arr)[sz])
    {
        UNREFERENCED_PARAMETER(arr);
        return sz;
    }

    /*
    * @brief : Use QueryDisplayConfig() to get paths, and modes.
    * @param[out] pathsV : reference to a vector which will contain paths
    * @param[out] modesV : reference to a vector which will contain modes
    * @param[in] flags : determines the kind of paths to retrieve (only active paths by default)
    * return : false in case of failure, else true
    */
    static bool GetPathsAndModes(std::vector<DISPLAYCONFIG_PATH_INFO>& pathsV, std::vector<DISPLAYCONFIG_MODE_INFO>& modesV, int flags = QDC_ONLY_ACTIVE_PATHS);

    //out own enum, similar to DISPLAYCONFIG_DEVICE_INFO_TYPE enum in wingdi.h
    enum class DISPLAYCONFIG_DEVICE_INFO_TYPE_CUSTOM : int
    {
        DISPLAYCONFIG_DEVICE_INFO_GET_DPI_SCALE = -3, //returns min, max, suggested, and currently applied DPI scaling values.
        DISPLAYCONFIG_DEVICE_INFO_SET_DPI_SCALE = -4, //set current dpi scaling value for a display
    };

    /*
    * struct DISPLAYCONFIG_SOURCE_DPI_SCALE_GET
    * @brief used to fetch min, max, suggested, and currently applied DPI scaling values.
    * All values are relative to the recommended DPI scaling value
    * Note that DPI scaling is a property of the source, and not of target.
    */
    struct DISPLAYCONFIG_SOURCE_DPI_SCALE_GET
    {
        DISPLAYCONFIG_DEVICE_INFO_HEADER            header;
        /*
        * @brief min value of DPI scaling is always 100, minScaleRel gives no. of steps down from recommended scaling
        * eg. if minScaleRel is -3 => 100 is 3 steps down from recommended scaling => recommended scaling is 175%
        */
        std::int32_t minScaleRel;

        /*
        * @brief currently applied DPI scaling value wrt the recommended value. eg. if recommended value is 175%,
        * => if curScaleRel == 0 the current scaling is 175%, if curScaleRel == -1, then current scale is 150%
        */
        std::int32_t curScaleRel;

        /*
        * @brief maximum supported DPI scaling wrt recommended value
        */
        std::int32_t maxScaleRel;
    };

    /*
    * struct DISPLAYCONFIG_SOURCE_DPI_SCALE_SET
    * @brief set DPI scaling value of a source
    * Note that DPI scaling is a property of the source, and not of target.
    */
    struct DISPLAYCONFIG_SOURCE_DPI_SCALE_SET
    {
        DISPLAYCONFIG_DEVICE_INFO_HEADER            header;
        /*
        * @brief The value we want to set. The value should be relative to the recommended DPI scaling value of source.
        * eg. if scaleRel == 1, and recommended value is 175% => we are trying to set 200% scaling for the source
        */
        int32_t scaleRel;
    };

    /*
    * struct DPIScalingInfo
    * @brief DPI info about a source
    * mininum :     minumum DPI scaling in terms of percentage supported by source. Will always be 100%.
    * maximum :     maximum DPI scaling in terms of percentage supported by source. eg. 100%, 150%, etc.
    * current :     currently applied DPI scaling value
    * recommended : DPI scaling value reommended by OS. OS takes resolution, physical size, and expected viewing distance
    *               into account while calculating this, however exact formula is not known, hence must be retrieved from OS
    *               For a system in which user has not explicitly changed DPI, current should eqaul recommended.
    * bInitDone :   If true, it means that the members of the struct contain values, as fetched from OS, and not the default
    *               ones given while object creation.
    */
    struct DPIScalingInfo
    {
        UINT32 mininum = 100;
        UINT32 maximum = 100;
        UINT32 current = 100;
        UINT32 recommended = 100;
        bool bInitDone = false;
    };

    DpiHelper();
    ~DpiHelper();
    static DpiHelper::DPIScalingInfo GetDPIScalingInfo(LUID adapterID, UINT32 sourceID);
    static bool SetDPIScaling(LUID adapterID, UINT32 sourceID, UINT32 dpiPercentToSet);
};

