/*
 * Startup functions. Finding devices, controlling logs, etc
 */
#include "../ftdi_lib/ftd2xx.h"
#include <stdint.h>
#include <stdio.h>


typedef struct{
    int device_type;
    int max_pos;
    int max_vel;
    int max_acc;
} device;

typedef struct {
    int32_t SN;
    uint8_t device_type;
    uint8_t HWtype;
    uint8_t dest = 0x50;
    uint8_t source = 0x01;
    uint8_t chanID = 0x01;
    uint16_t chan = 0x0001;
    uint8_t update_rate = 0x0A;
} defaults;

int addVidPid(){
    //not implemented, dmesg || /var/log/kern.log, add all devicess
    return -1;
}

int checkModules(){
    //check for ftdi_sio, usbserial, cannot be loaded!!!
    //not implemented 
    return -1;
}

int CheckLog(){
    //not implemented;
    return -1;
}

int CheckCron(){
    //not implemented;
    return -1;
}

int OpenDevice(){
    //not implemented, FT_OpenEx, set communication
    return -1;
}

int LoadRestrictions(){
    //not implemented
    return -1;
}

int LoadDefaults(){
    //not implemented
    return -1;
}

int init(){
    checkModules();
    addVidPid();
    
    FT_STATUS ft_status;
    unsigned int numdevs = 0;
    ft_status =  FT_CreateDeviceInfoList(&numdevs);
    if ( ft_status != FT_OK ) printf("FAIL\n");
    
    //not implemented yet
    return -1;
}