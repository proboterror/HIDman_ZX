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

// Number Input, Usage Page, Input Usage, Output Channel, Output Control, InputType, Input Param
__code JoyPreset KempstonJoyMaps[] = {
    // Buttons
    { 1, REPORT_USAGE_PAGE_BUTTON, 1, MAP_KEMPSTON, MAP_KEMPSTON_BUTTON1, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 2, MAP_KEMPSTON, MAP_KEMPSTON_BUTTON2, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 3, MAP_KEMPSTON, MAP_KEMPSTON_BUTTON3, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 4, MAP_KEMPSTON, MAP_KEMPSTON_BUTTON4, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    // D-Pad / Left Analog
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_X, MAP_KEMPSTON, MAP_KEMPSTON_R, MAP_TYPE_THRESHOLD_ABOVE, 192 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_X, MAP_KEMPSTON, MAP_KEMPSTON_L, MAP_TYPE_THRESHOLD_BELOW, 64 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_Y, MAP_KEMPSTON, MAP_KEMPSTON_D, MAP_TYPE_THRESHOLD_ABOVE, 192 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_Y, MAP_KEMPSTON, MAP_KEMPSTON_U, MAP_TYPE_THRESHOLD_BELOW, 64 },
    // POV/HAT switch
    // 0 on hat switch, just press up
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_U, MAP_TYPE_EQUAL, 0 },
    // 1 on hatswitch, press up and right
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_U, MAP_TYPE_EQUAL, 1 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_R, MAP_TYPE_EQUAL, 1 },
    // 2 on hatswitch, press right
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_R, MAP_TYPE_EQUAL, 2 },
    // 3 on hat, press right and down
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_R, MAP_TYPE_EQUAL, 3 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_D, MAP_TYPE_EQUAL, 3 },
    // 4 on hat, press down
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_D, MAP_TYPE_EQUAL, 4 },
    // 5 on hat, press down and lef
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_D, MAP_TYPE_EQUAL, 5 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_L, MAP_TYPE_EQUAL, 5 },
    // 6 on hat, press left
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_L, MAP_TYPE_EQUAL, 6 },
    // 7 on hat, press left and up
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_L, MAP_TYPE_EQUAL, 7 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEMPSTON, MAP_KEMPSTON_U, MAP_TYPE_EQUAL, 7 },
    // null to signify end
    { 0, 0, 0, 0, 0, 0, 0 }
};

__code JoyPreset DefaultJoyMaps[] = {
    { 1, REPORT_USAGE_PAGE_BUTTON, 1, MAP_KEYBOARD, KEY_A, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 2, MAP_KEYBOARD, KEY_B, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 3, MAP_KEYBOARD, KEY_C, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 4, MAP_KEYBOARD, KEY_D, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 5, MAP_KEYBOARD, KEY_E, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 6, MAP_KEYBOARD, KEY_F, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 7, MAP_KEYBOARD, KEY_G, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 8, MAP_KEYBOARD, KEY_H, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 9, MAP_KEYBOARD, KEY_ESC, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 10, MAP_KEYBOARD, KEY_ENTER, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 11, MAP_KEYBOARD, KEY_I, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 12, MAP_KEYBOARD, KEY_J, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 13, MAP_KEYBOARD, KEY_K, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 14, MAP_KEYBOARD, KEY_L, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 15, MAP_KEYBOARD, KEY_M, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_BUTTON, 16, MAP_KEYBOARD, KEY_N, MAP_TYPE_THRESHOLD_ABOVE, 0 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_X, MAP_KEYBOARD, KEY_RIGHT, MAP_TYPE_THRESHOLD_ABOVE, 192 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_X, MAP_KEYBOARD, KEY_LEFT, MAP_TYPE_THRESHOLD_BELOW, 64 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_Y, MAP_KEYBOARD, KEY_DOWN, MAP_TYPE_THRESHOLD_ABOVE, 192 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_Y, MAP_KEYBOARD, KEY_UP, MAP_TYPE_THRESHOLD_BELOW, 64 },
    // 0 on hat switch, just press up
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_UP, MAP_TYPE_EQUAL, 0 },
    // 1 on hatswitch, press up and right
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_UP, MAP_TYPE_EQUAL, 1 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_RIGHT, MAP_TYPE_EQUAL, 1 },
    // 2 on hatswitch, press right
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_RIGHT, MAP_TYPE_EQUAL, 2 },
    // 3 on hat, press right and down
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_RIGHT, MAP_TYPE_EQUAL, 3 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_DOWN, MAP_TYPE_EQUAL, 3 },
    // 4 on hat, press down
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_DOWN, MAP_TYPE_EQUAL, 4 },
    // 5 on hat, press down and left
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_DOWN, MAP_TYPE_EQUAL, 5 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_LEFT, MAP_TYPE_EQUAL, 5 },
    // 6 on hat, press left
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_LEFT, MAP_TYPE_EQUAL, 6 },
    // 7 on hat, press left and up
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_LEFT, MAP_TYPE_EQUAL, 7 },
    { 1, REPORT_USAGE_PAGE_GENERIC, REPORT_USAGE_HATSWITCH, MAP_KEYBOARD, KEY_UP, MAP_TYPE_EQUAL, 7 },
    // null to signify end
    { 0, 0, 0, 0, 0, 0, 0 }
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
