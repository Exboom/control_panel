#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <cstdint>

typedef struct _HID_JOYSTK_Info
{
    uint16_t              X;
    uint16_t              Y;
    uint16_t              Z;
    uint16_t              X_low;
    uint16_t              Y_low;
    uint8_t               buttons[5];
}
HID_JOYSTK_Info_TypeDef;

#endif // TYPEDEFS_H
