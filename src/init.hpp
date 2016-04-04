/*
 * Startup functions. Finding devices, controlling logs, etc
 */
#include "../ftdi_lib/ftd2xx.h"
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <climits>
#include <dirent.h>
#include <vector>
#include "device.hpp"
#include "api_calls.hpp"
#include "log.hpp"

#define NO_RESTRICTIONS 2
#define STOP 1
#define SYSTEM_ERROR -3
#define FIND_DEV(code) if (strncmp(name.c_str(), #code, 6) == 0){ printf("Found controller device, type: %s\n", name.c_str()) ; return code;}

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
        return SYSTEM_ERROR;
    }
    
    char buff[64];
    if (fgets(buff, 64, out) != NULL){
        string rmmod_cmd = "rmmod ";
        rmmod_cmd.append(module_name);
        if (system(rmmod_cmd.c_str()) != 0) return SYSTEM_ERROR;
        printf("Removing loaded module : %s \n",module_name.c_str());
    }
    
    if( pclose(out) == -1 ){ 
        printf("Failed to close input from system. \n");
        return SYSTEM_ERROR;
    }
    return 0;
}

int LoadRestrictions(controller_device &device){
    //not implemented
    if( device.channels != -1 ){
        for (int i = 0; i< device.channels; i++){
            device.motor[i].max_acc = INT_MAX;
            device.motor[i].max_pos = INT_MAX;
            device.motor[i].max_vel = INT_MAX;
        }
        return NO_RESTRICTIONS;    
    } 
    else {
        for(int i = 0; i< device.bays; i++){
            if(device.bay_used[i] == false) continue;
            device.motor[i].max_acc = INT_MAX;
            device.motor[i].max_pos = INT_MAX;
            device.motor[i].max_vel = INT_MAX;
        }
        return NO_RESTRICTIONS;        
    }
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
    FIND_DEV(TBD001)
    return -1;
}
int Bays(int type){
    if (type == BBD101 || type == BBD201 || type == BSC201) return 1;
    if (type == BBD102 || type == BBD202 || type == BSC202) return 2;
    if (type == BSC103 || type == BBD103  || type == BSC203  || type== BBD203) return 3;
    return -1;
}

int Channels(int type){
    if ( type == BSC001 || type == BMS001 || type == BSC101 || type == OST001 || type == ODC001
      || type == TST001 || type == TDC001 || type == TSC001 || type == TBD001 ) return 1;
    
    if ( type == BSC002 || type == BMS002 || type == MST601 || type == MST601 || type == BSC102 ) return 2;
    
    return -1;
};

int LoadDeviceInfo( controller_device &device){
    device.ft_opened = true;
    device.dest = 0x11;
    int ret;
    ret = device_calls::FlashProgNo(device.dest, 0x01);
    if (ret != 0) return ret;
    
    HwInfo *info = (HwInfo*) malloc(sizeof(HwInfo));
    ret = device_calls::GetHwInfo( info, 0x50, 0x01);
    if (ret != 0) return ret;
    device.hw_type = info->HWType();
    std::string model_num = info->ModelNumber();
    device.device_type = ToDevType(model_num);
    if (device.device_type == -1 ){ 
        printf("WARNING: unrecognized controller device\n");
        printf("Select device manually? Y/n\n");
        std::string option;
        cin >> option;
        if (option.compare("Y")==0 || option.compare("y") == 0 ){
            printf("Insert device code name (example \"TDC001\")\n");
            cin >> model_num;
            device.device_type = ToDevType(model_num);
            if (device.device_type == -1) printf("Unrecognized by program\n");
        }
        else return STOP;
    }
    free(info);
    
    device.channels = Channels(device.device_type);
    device.bays = Bays(device.device_type);
    
    if (device.bays != -1){
        for (uint8_t i = 0; i< device.bays; i++){
            GetRackBayUsed *bayused =  (GetRackBayUsed*) malloc(sizeof(GetRackBayUsed));
            ret = device_calls::GetBayUsed(bayused, i, device.dest, 0x01 );
            if (ret != 0) return ret;
            if (bayused->GetBayState() == 1) device.bay_used[i]=true;
            else device.bay_used[i]=false;
            free (bayused);
        }
    }
    
    if ( LoadRestrictions(device) == NO_RESTRICTIONS ) 
        printf("WARNING: no restrictions used on acceleration and velocity\n");

    return 0;
}

int init(){
    int ret;
    printf("Starting.\n");
    LOG("Starting\n")
    if( RemoveModules("ftdi_sio")  != 0){ 
        LOG("Failed to remove module\n")
        return SYSTEM_ERROR;
    }
    if( RemoveModules("usbserial")  != 0){
        LOG("Failed to remove module\n")
        return SYSTEM_ERROR;
    }
    LOG("Modules checked/unloaded\n");
    ret = addVidPid();
    if (ret != 0 ){ 
        LOG("Loading devices failed\n");
        return ret;
    }
    
    devices_connected = SN.size();
    if (devices_connected == 0) {
        printf("No Thorlabs device found\n");
        LOG("No Thorlabs device found\n")
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
        free(ftdi_devs);
        return FT_ERROR;
    }
    
    //Find additional info to Thorlabs devices
    for (unsigned int j = 0; j<  devices_connected; j++){
        for (unsigned int i = 0; i< num_ftdi_devices; i++){
            if ( strncmp(ftdi_devs[i].SerialNumber, connected_device[j].SN, 8) == 0 ){
                FT_HANDLE handle;
                ft_status = FT_OpenEx( connected_device[j].SN, FT_OPEN_BY_SERIAL_NUMBER, &handle);
                if (ft_status != FT_OK ) { free(ftdi_devs); return FT_ERROR; }
                
                if (FT_SetBaudRate(handle, 115200) != FT_OK) return FT_ERROR;
                if (FT_SetDataCharacteristics(handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE) != FT_OK ) return FT_ERROR;
                usleep(50);
                if (FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX) != FT_OK ) return FT_ERROR;
                usleep(50);
                if (FT_SetFlowControl(handle, FT_FLOW_RTS_CTS, 0, 0) != FT_OK) return FT_ERROR;
                if (FT_SetRts(handle) != FT_OK ) return FT_ERROR;
                usleep(50);
                
                OpenDevice(j);
                opened_device.handle = handle;
                ret = LoadDeviceInfo(connected_device[j]);
                if (ret != 0 ){ free(ftdi_devs); return ret; }
            }
        }
    }
    LOG("Info for devices loaded\n");
    opened_device = connected_device[0];
    free(ftdi_devs);        
    return 0;
}

void exit(){
    free(connected_device);
    for (unsigned int i = 0; i < devices_connected; i++){
        FT_Close(connected_device[i].handle);
    }
    LOG("Exiting\n")
    if (use_log) LogEnd();
    printf("EXiting\n");
}