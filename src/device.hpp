
typedef struct{
    int device_type;
    int channels;
    int max_pos;
    int max_vel;
    int max_acc;
} device;

device *connected_device;
device opened_device;

// enum for device types
enum {
    BSC001,
    BSC002,
    BMS001,
    BMS002,
    MST601,
    MST602,
    BSC101,
    BSC102,
    BSC103,
    BSC201,
    BSC202,
    BSC203,
    BBD101,
    BBD102,
    BBD103,
    BBD201,
    BBD202,
    BBD203,
    OST001,
    ODC001,
    TST001,
    TDC001,
    TSC001,
    TDIxxx
};



