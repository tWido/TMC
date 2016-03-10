
typedef struct {
    int motor_type;
    int max_pos;
    int max_vel;
    int max_acc;
    int tick;
    int enc_count;
    uint8_t dest;
} motor_device;

typedef struct{
    bool ft_opened;
    int device_type;
    int channels;
    int in_hub;
    uint8_t dest;
    motor_device motor[3];
} controller_device;

int devices_connected;
controller_device *connected_device;
controller_device opened_device;

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



