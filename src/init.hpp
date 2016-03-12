/*
 * Startup functions. Finding devices, controlling logs, etc
 */
#include "../ftdi_lib/ftd2xx.h"
#include "device.hpp"
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <set>
#define USB_PATH path = dev_path; path.append(loc);

using namespace std;

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
    string dev_path = "/sys/bus/usb/devices/";
    string vendor_path = "/idVendor";
    string product_path = "/idProduct";
    string serial_path = "/serial";
    string manufacturer_path = "/manufacturer";
    string path;
    char *p;
    FT_STATUS ftStatus;
    
    FILE *bus_loc = popen("dmesg | grep Thorlabs | cut -d':' -f1 |cut -b20-", "r");
    char loc_buff[10];
    while (true ){
        if ( fgets(loc_buff, 10, bus_loc) == NULL ) break;
        string loc(loc_buff);
        const char c = '\0';
        loc.replace(loc.length()-1, 1, &c  );
        
        USB_PATH
        path.append(manufacturer_path);
        FILE *manufacturer = fopen(path.c_str(),"r");
        if (manufacturer == NULL) continue;                 //device disconnected
        else {
            char m_buff[25];
            fgets(m_buff, 20, manufacturer);
            if ( strncmp(m_buff, "Thorlabs", 8)!= 0 ) continue;  //other device connected in port
        }
        fclose(manufacturer);
        
        USB_PATH
        path.append(vendor_path);
        FILE *vid_file = fopen(path.c_str(),"r");
        if (vid_file == NULL) return -1;
        char vid_buff[5];
        if ( fgets(vid_buff, 5, vid_file) == NULL) return -1;
        unsigned int vid = strtol(vid_buff, &p, 16); 
        fclose(vid_file);
        
        USB_PATH
        path.append(product_path);
        FILE *pid_file = fopen(path.c_str(),"r");
        if (pid_file == NULL) return -1;
        char pid_buff[5];
        if ( fgets(pid_buff, 5, pid_file) == NULL) return -1;
        unsigned int pid = strtol(pid_buff, &p, 16); 
        fclose(pid_file);
        
        printf("Found Thorlabs device, vendor ID: 0x%s, product ID: 0x%s\n", vid_buff, pid_buff);
        ftStatus = FT_SetVIDPID(vid, pid);
        if (ftStatus != FT_OK ) {
            printf("Setting found vendor ID and product ID failed, error: %d ", ftStatus );
            return -1;
        }
            
        USB_PATH
        path.append(serial_path);
        FILE *serial = fopen(path.c_str(),"r");
        if (serial == NULL) return -1;
        //get sn
        fclose(serial);
    }
    
//    FILE* vendors = popen("dmesg | grep Thorlabs -A 1 -B 3 | grep idVendor | cut -d'=' -f2 | cut -b1-4", "r");
//    if (vendors == NULL){
//        printf("Failed to run system command. Modules not checked. \n");
//        return -1;
//    }
//    
//    FILE* products = popen("lsusb | grep \"Future Technology Devices International\" | grep \"Bus\" | cut -d':' -f3 | cut -b-4", "r");
//    if (products == NULL){
//        printf("Failed to run system command. Modules not checked. \n");
//        return -1;
//    }
//    
//    FT_STATUS ftStatus;
//    char vid_buff[5];
//    char pid_buff[5];
//    while (true ){
//        if ( fgets(pid_buff, 5, products) == NULL || fgets(vid_buff, 5, vendors) == NULL ) break;
//        
//        char *p, *pr;
//        unsigned int pid = strtol(pid_buff, &p, 16); 
//        unsigned int vid = strtol(vid_buff, &pr, 16);
//        if (*p != 0 || *pr != 0) {
//            continue;
//        }
//        printf("found Thorlabs device, vendor ID: 0x%s, product ID: 0x%s\n", vid_buff, pid_buff);
//        ftStatus = FT_SetVIDPID(vid, pid);
//        if (ftStatus != FT_OK ) {
//            printf("Setting found vendor ID and product ID failed, error: %d ", ftStatus );
//            return -1;
//        }
//    }
//    
//    if( pclose(vendors) == -1 ){ 
//        printf("Failed to close input from system. \n" );
//        return -1;
//    }
//    if( pclose(products) == -1 ){ 
//        printf("Failed to close input from system. \n") ;
//        return -1;
//    }
    
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
    std::set<std::string> serial_numbers;
    FT_STATUS ftStatus;
    unsigned int numdevs = 0;
    ftStatus =  FT_CreateDeviceInfoList(&numdevs);
    if (ftStatus != FT_OK) {
        printf("creating device list failed, err: %d \n", ftStatus );
        return -1;
    }
    printf("Devices found: %d\n", numdevs );
    connected_device = (controller_device *) malloc(numdevs*sizeof(controller_device));
    devices_connected= numdevs;
    
    FILE* SN = popen("dmesg | grep Thorlabs -A 1 -B 3 | grep SerialNumber: | cut -d':' -f3 | cut -b2-9", "r");
        if (SN == NULL){
        printf("Failed to run system command. SerialNumber not found. \n");
        return -1;
    }
    char buff[9];
    while ( fgets(buff, 9, SN) != NULL){
        std::string serial(buff);
        if (serial.length() == 1) continue; //SN file contains empty lines
        serial_numbers.insert(serial);
        std::cout<<serial<<endl;
    }
    //disconnected devices?
    //insert sn in connected device list;
    
    if( pclose(SN) == -1 ){ 
        printf("Failed to close input from system. \n");
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
    if (addVidPid() != 0) return -1;
    
    LoadSN();
    

    
    //not implemented yet
    return -1;
}