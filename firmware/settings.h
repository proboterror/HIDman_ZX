#pragma once

#include <stdbool.h>

typedef struct Settings {

    // Set all USB keyboards to run in Report Mode rather than Boot Mode
    bool KeyboardReportMode;

    // Set all USB mice to run in Report Mode rather than Boot Mode
    bool MouseReportMode;

    // Output debug info to serial port
    bool SerialDebugOutput;

} Settings;

extern __xdata Settings HMSettings;

void InitSettings();