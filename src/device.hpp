#ifndef DEVICE
#define DEVICE

#include <set>

typedef std::set<int> functions_set;

typedef struct {
    int motor_type;
    int max_pos;
    int max_vel;
    int max_acc;
    int tick;
    uint8_t dest;
    int32_t status_position;
    int16_t status_velocity;
    int32_t status_enc_count;
    uint16_t status_status_bits;
    bool moving = false;
    bool homing = false;
    bool stopping = false;
} motor_device;

typedef struct{
    FT_HANDLE handle;
    int device_type;
    int hw_type;
    int bays;      
    int channels;
    uint8_t dest;
    bool status_updates;
    char *SN =(char*) "00000000";
    char *dev_type_name =(char*) "No device name";
    bool bay_used[3];
    int enc_counter;     //controllers use either encoder count(1) or microsteps(0), unspecified (-1)
    motor_device motor[3];
    bool end_of_move_messages = true;
    functions_set functions;
} controller_device;

extern int devices_connected;
extern controller_device *connected_device;
extern controller_device opened_device;
extern int opened_device_index;

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
    TBD001
};



 #endif 

