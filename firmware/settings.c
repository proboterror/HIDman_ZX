#include "settings.h"

__xdata Settings HMSettings;

void InitSettings()
{
    HMSettings.KeyboardReportMode = false;
    HMSettings.MouseReportMode = true;
#ifdef DEBUG
    HMSettings.SerialDebugOutput = true;
#else
    HMSettings.SerialDebugOutput = false;
#endif
}