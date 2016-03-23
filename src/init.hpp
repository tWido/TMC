/*
 * Startup functions. Finding devices, controlling logs, etc
 */
#include "../ftdi_lib/ftd2xx.h"
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <dirent.h>
#include <vector>
#include "device.hpp"
#include "api_calls.hpp"

#define STOP 1
#define SYSTEM_ERROR -3
#define FIND_DEV(code) if (name.compare(#code)== 0){ printf("Found controller device, type: %s", name.c_str()) ; return code;}

#define USB_PATH path = dev_path; path.append(loc);

using namespace std;

vector<string> SN;

int addVidPid(){
    char *p;
    FT_STATUS ftStatus;
    string usb_path= "/sys/bus/usb/devices/";
    
    DIR *usb_devs = opendir(usb_path.c_str());
    if (usb_devs == NULL ) {
        printf("Error while opening usb devices directory %s", strerror(errno));
        return SYSTEM_ERROR;
    }
    struct dirent *st_dir;
    while ( (st_dir = readdir (usb_devs)) ){
        string file_path= usb_path;
        file_path.append(st_dir->d_name);
        file_path.append("/manufacturer");
        FILE *file_man = fopen(file_path.c_str(), "r");
        if (file_man == NULL) continue;
        char fread_buff[50];
        fgets(fread_buff, 50, file_man);
        fclose(file_man);
        if ( (strncmp(fread_buff,"Thorlabs", 8)==0) ){
            string vid_path = usb_path;
            vid_path.append(st_dir->d_name);
            vid_path.append("/idVendor");
            FILE *read_file = fopen(vid_path.c_str() ,"r");
            if (read_file == NULL) {
                printf("Error while reading vendor id %s", strerror(errno));
                closedir(usb_devs);
                return SYSTEM_ERROR;
            }
            fgets(fread_buff, 5, read_file);
            fclose(read_file);
            unsigned int vid = strtol(fread_buff, &p, 16);
            
            string pid_path = usb_path;
            pid_path.append(st_dir->d_name);
            pid_path.append("/idProduct");
            read_file = fopen(pid_path.c_str() ,"r");
            if (read_file == NULL) {
                printf("Error while reading product id %s", strerror(errno));
                closedir(usb_devs);
                return SYSTEM_ERROR;
            }
            fgets(fread_buff, 5, read_file);
            fclose(read_file);
            unsigned int pid = strtol(fread_buff, &p, 16);
            
            ftStatus = FT_SetVIDPID(vid, pid);
            if (ftStatus != FT_OK ) {
                printf("Setting found vendor ID and product ID failed, error: %d ", ftStatus );
                closedir(usb_devs);
                return FT_ERROR;
            }
            
            string sn_path = usb_path;
            sn_path.append(st_dir->d_name);
            sn_path.append("/serial");
            read_file = fopen(sn_path.c_str() ,"r");
            if (read_file == NULL) {
                printf("Error while reading product serial %s", strerror(errno));
                closedir(usb_devs);
                return SYSTEM_ERROR;
            }
            fgets(fread_buff, 9, read_file); 
            fclose(read_file);
           
            SN.push_back(string(fread_buff));
            printf("Found Thorlabs device vendor id: %d , product id: %d, serial: %s\n", vid, pid, fread_buff);
        }
        else continue;
    }
    closedir(usb_devs);
    
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


int CheckLog(){
    //not implemented;
    return -1;
}

int CheckCron(){
    //not implemented;
    return -1;
}

int LoadRestrictions(FT_HANDLE &handle, controller_device &device){
    // for connected motors load restrictions
    //not implemented
    return -1;
}

int ToDevType(std::string name){
    FIND_DEV(BSC001)
    FIND_DEV(BSC002)
    FIND_DEV(BMS001)
    FIND_DEV(BMS002)
    FIND_DEV(MST601)
    FIND_DEV(MST602)
    FIND_DEV(BSC101)
    FIND_DEV(BSC102)
    FIND_DEV(BSC103)
    FIND_DEV(BSC201)
    FIND_DEV(BSC202)
    FIND_DEV(BSC203)
    FIND_DEV(BBD101)
    FIND_DEV(BBD102)
    FIND_DEV(BBD103)
    FIND_DEV(BBD201)
    FIND_DEV(BBD202)
    FIND_DEV(BBD203)
    FIND_DEV(OST001)
    FIND_DEV(ODC001)
    FIND_DEV(TST001)
    FIND_DEV(TDC001)
    FIND_DEV(TSC001)
    FIND_DEV(TDIxxx)
    FIND_DEV(TBD001)
    return -1;
}

int Channels(int type){
    if ( type == BSC001 || type == BMS001 || type == BSC101 || type == BSC201 || type == BBD101 || type == BBD201 
            || type == OST001 || type == ODC001 || type == TST001 || type == TDC001 || type == TSC001 || type == TBD001 ) return 1;
    
    if ( type == BSC002 || type == BMS002 || type == MST601 || type == MST601 || type == BSC102 || type == BSC202
            || type == BBD102 || type== BBD202) return 2;
    
    if ( type == BSC103 || type == BSC203 || type == BBD103 || type== BBD203 ) return 3;
    return 0;
};

int LoadDeviceInfo(FT_HANDLE &handle, controller_device &device){
    device.ft_opened = true;
    device.handle = &handle;
    int ret;
    ret = device_calls::FlashProgNo(handle, device, 0x50, 0x01);
    if (ret != 0) return ret;
    
    HwInfo *info = NULL;
    ret = device_calls::GetHwInfo(handle, device, info, 0x50, 0x01);
    if (ret != 0) return ret;
    device.hw_type = info->HWType();
    device.dest = info->GetSource();
    std::string model_num = info->ModelNumber();
    device.device_type = ToDevType(model_num);
    if (device.device_type == -1 ) printf("WARNING: unrecognized controller device");// TODO manually get channels
    delete(info);
    
    device.channels = Channels(device.device_type);
    GetHubBayUsed *hub;
    ret = device_calls::GetHubUsed(handle, device, hub, device.dest);
    if (ret != 0) return ret;
    device.in_hub = hub->GetBayID();
    delete(hub);
    
    //motors connected
    LoadRestrictions(handle, device);
    //not implemented
    return -1;
}

int init(){
    printf("Starting.\n");
    if( RemoveModules("ftdi_sio")  != 0) return -1;
    if( RemoveModules("usbserial")  != 0) return -1;
    if (addVidPid() != 0) return -1;
    
    devices_connected = SN.size();
    if (devices_connected == 0) {
        printf("No Thorlabs device found. Exiting\n");
        return STOP;
    }
    connected_device = (controller_device*) malloc(  sizeof(controller_device) * devices_connected );
    for (unsigned int i = 0; i< SN.size(); i++) connected_device[i].SN = strdup(SN.at(i).c_str());
    
    FT_STATUS ft_status;
    unsigned int num_ftdi_devices;
    ft_status = FT_CreateDeviceInfoList(&num_ftdi_devices);
    if (ft_status != FT_OK) {
        printf("Detecting devices failed\n");
        return FT_ERROR;
    }
    
    FT_DEVICE_LIST_INFO_NODE *ftdi_devs = (FT_DEVICE_LIST_INFO_NODE*) malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) *num_ftdi_devices  ) ;  
    ft_status = FT_GetDeviceInfoList( ftdi_devs, &num_ftdi_devices) ;  
    if (ft_status != FT_OK) {
        printf("Detecting devices failed\n");
        return FT_ERROR;
    }
    
    //Find additional info to Thorlabs devices
    for (unsigned int j = 0; j<  devices_connected; j++){
        for (unsigned int i = 0; i< num_ftdi_devices; i++){
            if ( strncmp(ftdi_devs[i].SerialNumber, connected_device[j].SN, 8) == 0 ){
                FT_HANDLE handle;
                ft_status = FT_OpenEx( connected_device[j].SN, FT_OPEN_BY_SERIAL_NUMBER, &handle);
                if (ft_status != FT_OK ) return FT_ERROR; 
                LoadDeviceInfo(handle, connected_device[j]);
            }
        }
    }
    
    free(ftdi_devs);        
    //not implemented yet
    return -1;
}