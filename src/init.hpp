/*
 * Startup functions. Finding devices, initializing
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
#include <fstream>
#include <algorithm>
#include "device.hpp"
#include "api_calls.hpp"

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

void LoadNone(controller_device &device, int mot_id){
    device.motor[mot_id].max_acc = INT_MAX;
    device.motor[mot_id].max_pos = INT_MAX;
    device.motor[mot_id].max_vel = INT_MAX;
    return;
}

int LoadRestrictions(controller_device &device, std::string device_name){
    printf("Do you want to load restrictions for device %s, SN: %s? Y/N\n", device_name.c_str(), device.SN);
    std::string option;
    cin >> option;
    if (option.compare("y")== 0 || option.compare("Y")==0 ){
        int motors;
        if ( device.channels != -1 ) motors = device.bays;
        else motors = device.channels;
        for (int i = 0; i< motors; i++){
            printf("Restriction for motor in bay or channel %d\n", i);
            printf("Insert name of file in restrictions folder or N to load none\n");
            cin >> option;
            if (option.compare("N") == 0){ LoadNone(device,i); continue;}
            ifstream restr(option);
            while (!restr.is_open()){
                printf("File not found. Want to insert file name again? file name or N\n");
                cin >> option;
                if (option.compare("N") == 0){ LoadNone(device,i); break;}
                else restr.open(option.c_str(), ifstream::in);
            }
            if (restr.is_open()){
                string word;
                while (restr >> word ){
                    transform(word.begin(), word.end(),word.begin(), ::tolower);
                    if (word.compare("velocity")) {
                        restr >> word;
                        try {                
                            device.motor[i].max_vel = std::stoi(word, 0, 10);       
                        }                                        
                        catch(const std::exception& e) {         
                            printf("Contains invalid number. Velocity not loaded\n");
                        }
                    }
                    if (word.compare("position")) {
                        restr >> word;
                        try {                
                            device.motor[i].max_pos = std::stoi(word, 0, 10); 
                        }                                        
                        catch(const std::exception& e) {         
                            printf("Contains invalid number. Position not loaded\n");
                            device.motor[i].max_pos = INT_MAX;
                        }
                    }
                    if (word.compare("acceleration")) {
                        restr >> word;
                        try {                
                            device.motor[i].max_acc = std::stoi(word, 0, 10); 
                        }                                        
                        catch(const std::exception& e) {         
                            printf("Contains invalid number. Acceleration not loaded\n");
                        }
                    }
                }
            }
        }
    }
    else {
        if (device.channels == -1) for (int i =0; i< device.bays; i++) LoadNone(device, i);
        else for (int i =0; i< device.channels; i++) LoadNone(device, i);
    }
    return 0;
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

int ContainEncCounter(int type){
    if ( type == TBD001 || type == BBD101 || type == BBD102 || type == BBD103 
            || type == BBD201 || type == BBD202 || type == BBD203 ) return 1;
    
    if (type == TST001 || type == BSC103 || type == BSC201 || type == BSC202 || type == BSC203 || type == BSC102 ||
          type == TDC001 || type == BSC001 ||  type == BSC101 ||  type == BSC002 || type == MST601 || type == MST601  ) return 0;
    return -1;
}

int LoadDeviceInfo( controller_device &device){
    device.dest = 0x11;
    int ret;
    ret = device_calls::FlashProgNo(device.dest);
    if (ret != 0) return ret;
    
    HwInfo *info = (HwInfo*) malloc(sizeof(HwInfo));
    ret = device_calls::GetHwInfo( info, 0x50);
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
    device.enc_counter = ContainEncCounter(device.device_type);
            
    if (device.bays != -1){
        for (uint8_t i = 0; i< device.bays; i++){
            GetRackBayUsed *bayused =  (GetRackBayUsed*) malloc(sizeof(GetRackBayUsed));
            ret = device_calls::GetBayUsed(bayused, i, device.dest);
            if (ret != 0) return ret;
            if (bayused->GetBayState() == 1) device.bay_used[i]=true;
            else device.bay_used[i]=false;
            free (bayused);
        }
    }
    
    LoadRestrictions(device, model_num);

    return 0;
}

int init(){
    int ret;
    printf("Starting.\n");
    if( RemoveModules("ftdi_sio")  != 0){ 
        return SYSTEM_ERROR;
    }
    if( RemoveModules("usbserial")  != 0){
        return SYSTEM_ERROR;
    }
    ret = addVidPid();
    if (ret != 0 ){ 
        return ret;
    }
    
    devices_connected = SN.size();
    if (devices_connected == 0) {
        printf("No Thorlabs device found\n");
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
            if (OpenDevice(j) == FT_ERROR ) { free(ftdi_devs); return FT_ERROR; };
            if (ft_status != FT_OK ) { free(ftdi_devs); return FT_ERROR; }

            if (FT_SetBaudRate(opened_device.handle, 115200) != FT_OK) return FT_ERROR;
            if (FT_SetDataCharacteristics(opened_device.handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE) != FT_OK ) return FT_ERROR;
            usleep(50);
            if (FT_Purge(opened_device.handle, FT_PURGE_RX | FT_PURGE_TX) != FT_OK ) return FT_ERROR;
            usleep(50);
            if (FT_SetFlowControl(opened_device.handle, FT_FLOW_RTS_CTS, 0, 0) != FT_OK) return FT_ERROR;
            if (FT_SetRts(opened_device.handle) != FT_OK ) return FT_ERROR;
            usleep(50);
                              
            ret = LoadDeviceInfo(connected_device[j]);
            if (ret != 0 ){ free(ftdi_devs); return ret; }               
            }
        }
    
    free(ftdi_devs);  
    if (OpenDevice(0) == FT_ERROR) return FT_ERROR;
    return 0;
}

void exit(){
    free(connected_device);
    for (unsigned int i = 0; i < devices_connected; i++){
        FT_Close(opened_device.handle);
    }
    printf("Exiting\n");
}