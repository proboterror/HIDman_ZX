/*
    data.c
    
    Various tables - 
    Joystick mapping presets, Keyboard scancodes, HID->PS2 mappings, etc
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "usbhidkeys.h"
#include "data.h"
#include "scancode.h"

__code JoyPreset DefaultJoyMaps[] = {

    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        1,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_A,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        2,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_B,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        3,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_C,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        4,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_D,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        5,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_E,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        6,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_F,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        7,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_G,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        8,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_H,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        9,                        // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_ESC,                  // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        10,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_ENTER,                // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        11,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_I,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        12,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_J,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        13,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_K,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        14,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_L,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        15,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_M,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                        // Number
        REPORT_USAGE_PAGE_BUTTON, // Input Usage Page
        16,                       // Input Usage
        MAP_KEYBOARD,             // Output Channel
        KEY_N,                    // Output Control
        MAP_TYPE_THRESHOLD_ABOVE, // InputType
        0                         // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_X,            // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_RIGHT,                 // Output Control
        MAP_TYPE_THRESHOLD_ABOVE,  // InputType
        192                        // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_X,            // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_LEFT,                  // Output Control
        MAP_TYPE_THRESHOLD_BELOW,  // InputType
        64                         // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_Y,            // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_DOWN,                  // Output Control
        MAP_TYPE_THRESHOLD_ABOVE,  // InputType
        192                        // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_Y,            // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_UP,                    // Output Control
        MAP_TYPE_THRESHOLD_BELOW,  // InputType
        64                         // Input Param
    },
    // 0 on hat switch, just press up
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_UP,                    // Output Control
        MAP_TYPE_EQUAL,            // InputType
        0                          // Input Param
    },
    // 1 on hatswitch, press up and right
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_UP,                    // Output Control
        MAP_TYPE_EQUAL,            // InputType
        1                          // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_RIGHT,                 // Output Control
        MAP_TYPE_EQUAL,            // InputType
        1                          // Input Param
    },
    // 2 on hatswitch, press right
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_RIGHT,                 // Output Control
        MAP_TYPE_EQUAL,            // InputType
        2                          // Input Param
    },
    // 3 on hat, press right and down
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_RIGHT,                 // Output Control
        MAP_TYPE_EQUAL,            // InputType
        3                          // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_DOWN,                  // Output Control
        MAP_TYPE_EQUAL,            // InputType
        3                          // Input Param
    },
    // 4 on hat, press down
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_DOWN,                  // Output Control
        MAP_TYPE_EQUAL,            // InputType
        4                          // Input Param
    },
    // 5 on hat, press down and left
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_DOWN,                  // Output Control
        MAP_TYPE_EQUAL,            // InputType
        5                          // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_LEFT,                  // Output Control
        MAP_TYPE_EQUAL,            // InputType
        5                          // Input Param
    },
    // 6 on hat, press left
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_LEFT,                  // Output Control
        MAP_TYPE_EQUAL,            // InputType
        6                          // Input Param
    },
    // 7 on hat, press left and up
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_LEFT,                  // Output Control
        MAP_TYPE_EQUAL,            // InputType
        7                          // Input Param
    },
    {
        1,                         // Number
        REPORT_USAGE_PAGE_GENERIC, // Usage Page
        REPORT_USAGE_HATSWITCH,    // Usage
        MAP_KEYBOARD,              // Output Channel
        KEY_UP,                    // Output Control
        MAP_TYPE_EQUAL,            // InputType
        7                          // Input Param
    },
    // null to signify end
    {
        0, // Number
        0, // Input Usage Page
        0, // Input Usage
        0, // Output Channel
        0, // Output Control
        0, // InputType
        0  // Input Param
    },

};

__xdata uint8_t StandardKeyboardDescriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0, //   Usage Minimum (0xE0)
    0x29, 0xE7, //   Usage Maximum (0xE7)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x08, //   Report Count (8)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x08, //   Report Size (8)
    0x81, 0x01, //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x05, //   Report Count (5)
    0x75, 0x01, //   Report Size (1)
    0x05, 0x08, //   Usage Page (LEDs)
    0x19, 0x01, //   Usage Minimum (Num Lock)
    0x29, 0x05, //   Usage Maximum (Kana)
    0x91, 0x02, //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x03, //   Report Size (3)
    0x91, 0x01, //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x06, //   Report Count (6)
    0x75, 0x08, //   Report Size (8)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x65, //   Logical Maximum (101)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0x00, //   Usage Minimum (0x00)
    0x29, 0x65, //   Usage Maximum (0x65)
    0x81, 0x00, //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,       // End Collection

    // 63 bytes

};

__xdata uint8_t StandardMouseDescriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02, // Usage (Mouse)
    0xA1, 0x01, // Collection (Application)
    0x09, 0x01, //   Usage (Pointer)
    0xA1, 0x00, //   Collection (Physical)
    0x05, 0x09, //     Usage Page (Button)
    0x19, 0x01, //     Usage Minimum (0x01)
    0x29, 0x05, //     Usage Maximum (0x05)
    0x15, 0x00, //     Logical Minimum (0)
    0x25, 0x01, //     Logical Maximum (1)
    0x95, 0x05, //     Report Count (5)
    0x75, 0x01, //     Report Size (1)
    0x81, 0x02, //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01, //     Report Count (1)
    0x75, 0x03, //     Report Size (3)
    0x81, 0x01, //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01, //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30, //     Usage (X)
    0x09, 0x31, //     Usage (Y)
    0x15, 0x81, //     Logical Minimum (-127)
    0x25, 0x7F, //     Logical Maximum (127)
    0x75, 0x08, //     Report Size (8)
    0x95, 0x02, //     Report Count (2)
    0x81, 0x06, //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,       //   End Collection
    0xC0,       // End Collection

    // 50 bytes

};
