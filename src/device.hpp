
typedef struct{
    int device_type;
    uint8_t bay_connected[10];
    uint8_t dest = 0x11;
} hub_device;

typedef struct{
    bool ft_opened;
    int device_type;
    int channels;
    int max_pos;
    int max_vel;
    int max_acc;
    int tick;
    int enc_count;
    int in_hub;
    uint8_t dest;
    uint16_t chanID16;
    uint8_t chanID8;
} motor_device;

int devices_connected;
motor_device *connected_device;
motor_device opened_device;
hub_device *hub_devices;

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
    TDIxxx,
    TBD001
};



