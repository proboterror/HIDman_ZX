
#ifndef __SCANCODES_H__
#define __SCANCODES_H__
#include <stdbool.h>
#include <stdint.h>

extern __code uint8_t KEY_LEFT_BREAK[];
extern __code uint8_t KEY_RIGHT_BREAK[];
extern __code uint8_t KEY_UP_BREAK[];
extern __code uint8_t KEY_DOWN_BREAK[];

extern __code uint8_t KEY_LEFT_MAKE[];
extern __code uint8_t KEY_RIGHT_MAKE[];
extern __code uint8_t KEY_UP_MAKE[];
extern __code uint8_t KEY_DOWN_MAKE[];

extern __code uint8_t KEY_LCTRL_MAKE[];
extern __code uint8_t KEY_LALT_MAKE[];
extern __code uint8_t KEY_SPACE_MAKE[];
extern __code uint8_t KEY_ENTER_MAKE[];

extern __code uint8_t KEY_LCTRL_BREAK[];
extern __code uint8_t KEY_LALT_BREAK[];
extern __code uint8_t KEY_SPACE_BREAK[];
extern __code uint8_t KEY_ENTER_BREAK[];

extern __code uint8_t KEY_SET2_LSHIFT_MAKE[];
extern __code uint8_t KEY_SET2_LSHIFT_BREAK[];

extern const uint8_t * const HIDtoSET2_Make[];
extern const uint8_t * const HIDtoSET2_Break[];

#endif //__SCANCODES_H__