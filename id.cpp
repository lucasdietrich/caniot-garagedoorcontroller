#include "caniot/device.h"

// build_unflags =
//     -Wl,--gc-sections

// Set identification at location 0x77dc in flash
__attribute__((used, section(".device_identification"))) const can_device::identification_t _anonymous_id = {
    __DEVICE_ID__,
    __DEVICE_TYPE__,
    __FIRMWARE_VERSION__,
    __DEVICE_NAME__,
};