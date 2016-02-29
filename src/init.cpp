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
    //not implemented, dmesg ,  add all devicess
    return -1;
}

int RemoveModules(std::string module_name){
    std::string lsmod_cmd= "lsmod | grep ";
    lsmod_cmd.append(module_name);
    FILE* out = popen(lsmod_cmd.c_str(), "r");
    if (out == NULL){
        cout << "Failed to run system command. Modules not checked. \n";
        return -1;
    }
    
    char buff[128];
    if (fgets(buff, 128, out) != NULL){
        std::string rmmod_cmd = "rmmod ";
        rmmod_cmd.append(module_name);
        system(rmmod_cmd.c_str());
        cout << "Removing loaded module : " << module_name << "\n";
    }
    
    if( pclose(out) == -1 ){ 
        cout << "Failed to close input from system. \n" ;
        return -1;
    }
    return 0;
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