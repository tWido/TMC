/*
 * Startup functions. Finding devices, controlling logs, etc
 */
#include "../ftdi_lib/ftd2xx.h"
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <errno.h>

using namespace std;;

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
    FILE* vendors = popen("dmesg | grep Thorlabs -A 1 -B 3 | grep idVendor | cut -d'=' -f2 | cut -b1-4", "r");
    if (vendors == NULL){
        printf("Failed to run system command. Modules not checked. \n");
        return -1;
    }
    
    FILE* products = popen("dmesg | grep Thorlabs -A 1 -B 3 | grep idVendor | cut -d'=' -f3 | cut -b1-4", "r");
    if (products == NULL){
        printf("Failed to run system command. Modules not checked. \n");
        return -1;
    }
    
    //save pid & vid - possibly more
    char vid_buff[4];
    char pid_buff[4];
    while (true ){
        if ( fgets(pid_buff, 4, products) != NULL || fgets(pid_buff, 4, vendors) != NULL ) break;
        unsigned int pid = atoi(pid_buff); 
        unsigned int vid = atoi(vid_buff);
        FT_SetVIDPID(vid, pid);
    }
    
    if( pclose(vendors) == -1 ){ 
        printf("Failed to close input from system. \n" );
        return -1;
    }
    if( pclose(products) == -1 ){ 
        printf("Failed to close input from system. \n") ;
        return -1;
    }
    
    return 0;
}

int RemoveModules(std::string module_name){
    string lsmod_cmd= "lsmod | grep ";
    lsmod_cmd.append(module_name);
    FILE* out = popen(lsmod_cmd.c_str(), "r");
    if (out == NULL){
        printf("Failed to run system command. Modules not checked. \n");
        return -1;
    }
    
    char buff[64];
    if (fgets(buff, 64, out) != NULL){
        string rmmod_cmd = "rmmod ";
        rmmod_cmd.append(module_name);
        system(rmmod_cmd.c_str());
        printf("Removing loaded module : %s \n",module_name.c_str());
    }
    
    if( pclose(out) == -1 ){ 
        printf("Failed to close input from system. \n");
        return -1;
    }
    return 0;
}

int LoadSN(){
    FILE* out = popen("dmesg | grep Thorlabs -A 1 -B 3 | grep SerialNumber: | cut -d' ' -f7", "r");
        if (out == NULL){
        printf("Failed to run system command. Modules not checked. \n");
        return -1;
    }
    //read sn
    
    if( pclose(out) == -1 ){ 
        printf("Failed to close input from system. \n");
        return -1;
    }
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
    printf("Starting.\n");
    if( RemoveModules("ftdi_sio")  != 0) return -1;
    if( RemoveModules("usbserial")  != 0) return -1;
    
   // addVidPid();
    
  //  FT_STATUS ft_status;
 //   unsigned int numdevs = 0;
 //   ft_status =  FT_CreateDeviceInfoList(&numdevs);
  //  if ( ft_status != FT_OK ) printf("FAIL\n");
    
    //not implemented yet
    return -1;
}