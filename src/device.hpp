#include <stdint.h>

// enum for device types
enum {
    BBD101,
    BBD102,
    BBD103,
    BSC001,
    BMS001,
    BSC002,
    BMS002,
    OST001,
    ODC001,
    BSC103,
    TST001,
    BSC101,
    TDC001
};

typedef struct{
    int device_type;
    int channels;
    int max_pos;
    int max_vel;
    int max_acc;
} device;

device connected_device;