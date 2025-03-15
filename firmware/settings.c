#include "settings.h"

__xdata Settings HMSettings;

void InitSettings()
{
    HMSettings.KeyboardReportMode = false;
    HMSettings.MouseReportMode = false;
    HMSettings.GameControllerAsMouse = false;
#ifdef DEBUG
    HMSettings.SerialDebugOutput = true;
#else
    HMSettings.SerialDebugOutput = false;
#endif
}