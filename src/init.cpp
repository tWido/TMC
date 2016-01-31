/*
 * Startup functions. Finding devices, controlling logs, etc
 */
#include "../ftdi_lib/ftd2xx.h"
#include <stdint.h>
#include <stdio.h>


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

int CheckEndianess(){
    //not implemented, lscpu, messages in little endian
    return -1;
}

int LoadRestrictions(){
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