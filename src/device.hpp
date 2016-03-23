#ifndef DEVICE
#define DEVICE

typedef struct {
    int motor_type;
    int max_pos;
    int max_vel;
    int max_acc;
    int tick;
    int enc_count;
    uint8_t dest;
    bool status_updates = false;
    uint32_t status_position;
    uint16_t status_velocity;
    uint32_t status_enc_count;
    uint16_t status_status_bits;
    bool moving = false;
    bool homing = false;
    bool stopping = false;
} motor_device;

typedef struct{
    FT_HANDLE *handle;
    bool ft_opened;
    int device_type;
    int channels;
    int in_hub;
    uint8_t dest;
    char *SN;
    motor_device motor[3];
    bool end_of_move_messages = false;
} controller_device;

unsigned int devices_connected;
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

 #endif 

