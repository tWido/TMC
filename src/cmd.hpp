#ifndef CMD_HPP
#define	CMD_HPP
#include "api_calls.hpp"
#include "device.hpp"
#include <map>
#include <vector>
#include <string.h>
#include <iostream>
#include <string>

#define INVALID_CALL -1
#define HAS_FLAG(x) if ((connected_device[dev_index].motor[mot_index].status_status_bits & x) == x) 
#define CHANNEL_OPERATION(operation) if (args.size() <= i+1 ) {             \
                printf("Channel\\bay number not specified\n");              \
                return INVALID_CALL;                                        \
            }                                                               \
            try {                                                           \
            id = std::stoi(args.at(i+1), 0, 10);                            \
            }                                                               \
            catch(const std::exception& e) {                                \
                printf("Given argument is not a valid number\n");           \
                return INVALID_CALL;                                        \
            }                                                               \
            if (opened_device.bays == -1){                                  \
                if (operation(0x50,id) == INVALID_CHANNEL ){                \
                    printf("Not existing channel number given\n");          \
                    return 0;                                               \
                }                                                           \
            }                                                               \
            else {                                                          \
                id += 0x20;                                                 \
                if (operation(id) == INVALID_DEST ){                        \
                    printf("Not existing channel number given\n");          \
                    return 0;                                               \
                }                                                           \
            }                                                               \
            i++;


typedef int (*helper)(std::vector<string>);
typedef std::map<std::string,helper> call_map;

bool validation = false;

int HelpC(std::vector<string> args){
    if (args.size() > 1) printf("No arguments needed\n");
    printf(" help        prints this help message, for command info use -h\n");
    printf(" open        switch between controlled devices \n");
    printf(" devinfo     prints connected device information \n");
    printf(" flash       flashes front panel LED \n");
    printf(" chan        channel state \n");
    printf(" poscount    device's position counter \n");
    printf(" enccount    device's encoder counter \n");
    printf(" velp        acceleration and maximum velocity \n");
    printf(" jogp        jog move parameters \n");
    printf(" powerp      power used while motor moves or rests \n");
    printf(" bdist       backlash distance value \n");
    printf(" relmovep    relative move parameters \n");
    printf(" absmovep    absolute move parameters \n");
    printf(" limitsw     limit switch parameters \n");
    printf(" homingvel   homing velocity \n");
    printf(" home        move to home position \n");
    printf(" relmove     start relative move \n");
    printf(" absmove     start absolute move \n");
    printf(" jogmove     start jog move \n");
    printf(" velmove     start move with set velocity \n");
    printf(" stop        stop movement \n");
    printf(" ledp        front LED parameters \n");
    printf(" buttp       device's buttons parameters \n");
    printf(" status      get status \n");
    printf(" statusdc    get status for dc servo controller \n");
    printf(" eom         trigger parameters \n");
    return 0;
}

int OpenDeviceC(std::vector<string> args){
    if (args.size() == 1 ) {
        printf("No arguments\n");
        return INVALID_CALL;
    }
    if (args.size() > 3 ) {
        printf("Unexpected number of arguments, see -h for help\n");
        return INVALID_CALL;
    } 
    if (args.at(1) == "-h"){ 
        printf("Choose which connected device to control\n");
        printf("-n NUMBER       device number in list created by program, see devinfo\n");
        printf("-s SN           serial number of device\n");
        return 0;
    }
    if ( args.at(1).compare("-n") == 0 ) {
        if (args.size() == 2 ) {
            printf("Number not given\n");
            return INVALID_CALL;
        }
        unsigned int num = -1;
        try {
            num = std::stoi(args.at(2));
        }
        catch(const std::exception& e) { 
            printf("Given argument is not a valid number\n");
            return INVALID_CALL;
        }
        if (!validation) if (OpenDevice(num-1) != 0 ) printf("Incorrect device number\n");       //for user devices are numbered from 1
        return 0;
    }
    if ( args.at(1).compare("-s") == 0) {
        if (args.size() == 2 ) {
            printf("Serial not specified\n");
            return INVALID_CALL;
        }
        for ( unsigned int i = 0; i< devices_connected; i++ ){
            std::string to_comp (connected_device[i].SN);
            if ( to_comp.compare(args.at(2)) == 0 ) OpenDevice(i); 
        }
        if (!validation) printf("Device with specified serial number not present\n");
        return 0;
    }
    printf("Unrecognized parameter %s, see -h for help\n", args.at(1).c_str());
    return 0;
}

void DeviceInfoHelper(int dev_index, int mot_index){
    if (connected_device[dev_index].motor[mot_index].enc_count != 0) printf("  Encoder count : %d\n",connected_device[dev_index].motor[mot_index].enc_count);
        if (connected_device[dev_index].motor[mot_index].homing) printf("  Moving to home position\n");
        if (connected_device[dev_index].motor[mot_index].moving) printf("  Moving\n");
        if (connected_device[dev_index].motor[mot_index].stopping) printf("  Stopping\n");
        if (connected_device[dev_index].motor[mot_index].status_updates){
            if (connected_device[dev_index].motor[mot_index].enc_count != 0) printf("  Status message - encoder count : %d\n",connected_device[dev_index].motor[mot_index].status_enc_count);
            else printf("  Status message - position : %d\n",connected_device[dev_index].motor[mot_index].status_position);
            printf("  Status message - velocity : %d\n",connected_device[dev_index].motor[mot_index].status_velocity);
            printf("  Status bits info: \n");
            HAS_FLAG(0x00000001) printf("   Forward hardware limit switch active\n");
            HAS_FLAG(0x00000002) printf("   Reverse hardware limit switch active\n");
            HAS_FLAG(0x00000010) printf("   Moving forward\n");
            HAS_FLAG(0x00000020) printf("   Moving reverse\n");
            HAS_FLAG(0x00000040) printf("   Jogging forward\n");
            HAS_FLAG(0x00000080) printf("   Jogging reverse\n");
            HAS_FLAG(0x00000200) printf("   Homing\n");
            HAS_FLAG(0x00000400) printf("   Homed\n");
            HAS_FLAG(0x00001000) printf("   Tracking\n");
            HAS_FLAG(0x00002000) printf("   Settled\n");
            HAS_FLAG(0x00004000) printf("   Motion error\n");
            HAS_FLAG(0x01000000) printf("   Motor limit reached\n");
            HAS_FLAG(0x80000000) printf("   Channel enabled\n");
        }
}

int DeviceInfoC(std::vector<string> args){
    if (args.size() > 1 && args.at(1) == "-h"){ 
        printf("Prints information to all compatible devices connected\n");
        return 0;
    }
    if (args.size() > 1) printf("No arguments except -h\n");
    int prev_opened = opened_device_index;
    for (unsigned int i = 0; i< devices_connected; i++){
        OpenDevice(i);
        printf(" Device number %d\n", i+1);
        printf(" Serial: %s\n",connected_device[i].SN);
        if (connected_device[i].bays != -1 ){ 
            printf(" Bays: %d\n", connected_device[i].bays);
            for (int j = 0; j < connected_device[i].bays; j++ ){ 
                printf(" Motor in bay %d: ",j+1);                           //bays numbered from 1 for user
                if (connected_device[i].bay_used[j]) printf("Unused\n");
                else{
                    printf("Used\n");
                    printf("  Destination address : %d\n",connected_device[i].motor[j].dest);
                    DeviceInfoHelper(i,j);
                }
            }
        }
        if (connected_device[i].channels != -1 ){ 
            printf(" Channels: %d\n", connected_device[i].channels);
            printf(" Channel 1\n");
            DeviceInfoHelper(i,0);
            if (connected_device[i].channels == 2 ){
                printf(" Channel 2\n");
                DeviceInfoHelper(i,1);
            }
        }
        HwInfo *info = (HwInfo*) malloc(sizeof(HwInfo));
        if (device_calls::GetHwInfo(info) != 0) printf("Error occured while receiving info from device\n");
        printf("  Model number: %s\n",info->ModelNumber().c_str());
        printf("  Hardware version: %d\n", info->HwVersion());
        printf("  Notes: %s", info->Notes().c_str());
        free(info);
    }
    
    OpenDevice(prev_opened);
    return 0;
}

int IdentC(std::vector<string> args){
    if (args.size() == 1 ) {
        if (!validation) device_calls::Identify();
        return 0;
    }
    uint8_t dest;

    if (args.at(1) == "-h"){
        printf("Flashes front LED on device, optional destination parameter\n");
        printf("-d VALUE    optional parameter - destination address in decimal\n");
        printf("-D VALUE    optional parameter - destination address in hexadecimal\n");
        return 0;
    }
    if (args.at(1).compare("-d") == 0 ){
        if (args.size() == 2 ) {
            printf("Address not given\n");
            return INVALID_CALL;
        }
        try {
            dest = std::stoi(args.at(2), 0, 10);
        }
        catch(const std::exception& e) { 
            printf("Given argument is not a valid number\n");
            return INVALID_CALL;
        }
    }
    else if (args.at(1).compare("-D") == 0 ){
        if (args.size() == 2 ) {
            printf("Address not given\n");
            return INVALID_CALL;
        }
        try {
            dest = std::stoi(args.at(2), 0, 16);
        }
        catch(const std::exception& e) { 
            printf("Given argument is not a valid number\n");
            return INVALID_CALL;
        }
    }
    else printf("Unrecognized parameter %s, see -h for help\n", args.at(1).c_str());
    if (!validation) if (device_calls::Identify(dest) == INVALID_DEST) printf("Invalid destination given\n");
    return 0;
}

int ChannelAbleC(std::vector<string> args){
    for (unsigned int i = 0; i< args.size(); i++){
        int id;
        if (args.at(i).compare("-h") == 0){
            printf("Operating channels or bays, numbers range from 1 \n");
            printf("-e NUMBER    enable channel or bay with given number\n");
            printf("-d NUMBER    disable channel or bay with given number\n");
            printf("-i NUMBER    information about ability state for channel or bay at given position\n");
        }
        if (args.at(i).compare("-e") == 0){ CHANNEL_OPERATION(device_calls::EnableChannel) }
        if (args.at(i).compare("-d") == 0){ CHANNEL_OPERATION(device_calls::DisableChannel) }
        if (args.at(i).compare("-i") == 0){
            if (args.size() <= i+1 ) {             
                printf("Channel\\bay number not specified\n");              
                return INVALID_CALL;                                        
            }
            try {                                                           
            id = std::stoi(args.at(i+1), 0, 10);                            
            }                                                               
            catch(const std::exception& e) {                                
                printf("Given argument is not a valid number\n");           
                return INVALID_CALL;                                        
            }
            GetChannelState *state = (GetChannelState*) malloc(sizeof(GetChannelState));
            if (opened_device.bays == -1){     
                if (device_calls::ChannelState(state,0x50,id) != 0 ){                
                    printf("Error while receiving information from device\n");
                    free(state);
                    return 0;                                               
                }
                if (state->GetSecondParam() == 0x01 ) printf("Enabled\n");
                else printf("Disabled\n");
            }                                                               
            else {                                                          
                id += 0x20;                                                
                if (device_calls::ChannelState(state,id) != 0 ){                        
                    printf("Error while receiving information from device\n");
                    free(state);
                    return 0;  
                }
                if(state->GetSecondParam() == 0x01 ) printf("Enabled\n");
                else printf("Disabled\n");
            }
            free(state);
            i++; 
        }
    }
    return 0;
}

int PosCounterC(std::vector<string> args){
    if (args.at(1).compare("-h") == 0){
        printf("WARNING: \n");
        printf("Set or get actual position counter in device\n");
        printf("-s NUMBER    set\n");
        printf("-g           get\n");
    }
    return 0;
}

int EncCountC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int VelParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int JogParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int PowerParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int BacklashDistC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int RelMoveParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int AbsMoveParamC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int HomingVelC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int LimitSwitchC(std::vector<string> args){
    // set, get
    //not implemented
    return 0;
}

int HomeC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartRelMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartAbsMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartJogMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StartVelMoveC(std::vector<string> args){
    //not implemented
    return 0;
}

int StopC(std::vector<string> args){
    //not implemented
    return 0;
}

int AccParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

int LedParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

int ButtonsParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

int StatusC(std::vector<string> args){
    //GetStatBits, GetStatus
    //not implemented
    return 0;
}

int DCStatusC(std::vector<string> args){
    //not implemented
    return 0;
}

int TriggerParamC(std::vector<string> args){
    // get, set
    //not implemented
    return 0;
}

call_map calls = {
    std::make_pair("help", &HelpC),
    std::make_pair("flash", &IdentC),
    std::make_pair("chan", &ChannelAbleC),
    std::make_pair("open", &OpenDeviceC),
    std::make_pair("devinfo", &DeviceInfoC),
    std::make_pair("poscount", &PosCounterC),
    std::make_pair("enccount", &EncCountC),
    std::make_pair("velp", &VelParamC),
    std::make_pair("jogp", &JogParamC),
    std::make_pair("powerp", &PowerParamC),
    std::make_pair("bdist", &BacklashDistC),
    std::make_pair("relmovep", &RelMoveParamC),
    std::make_pair("absmovep", &AbsMoveParamC),
    std::make_pair("homingvel", &HomingVelC),
    std::make_pair("limitsw", &LimitSwitchC),
    std::make_pair("home", &HomeC),
    std::make_pair("relmove", &StartRelMoveC),
    std::make_pair("absmove", &StartAbsMoveC),
    std::make_pair("jogmove", &StartJogMoveC),
    std::make_pair("velmove", &StartVelMoveC),
    std::make_pair("stop", &StopC),
    std::make_pair("accp", &AccParamC),
    std::make_pair("ledp", &LedParamC),
    std::make_pair("buttp", &ButtonsParamC),
    std::make_pair("status", &StatusC),
    std::make_pair("statusdc", &DCStatusC),
    std::make_pair("triggp", &TriggerParamC)
};


int run_cmd(){
      
    while(true){
        std::string line = "";
        std::getline(std::cin, line);
        if (line == "" ) continue;
        
        std::vector<std::string> args;
        const char delimiter = ' ';
        char* token = strtok(strdup(line.c_str()), &delimiter);
        while(token != NULL){
            args.push_back(std::string(token));
            token = strtok(NULL, &delimiter);
        }
        
        if ( args.at(0).compare("exit") == 0 ) break;
        if ( calls.count(args.at(0))== 0 ) {
            printf("Unrecognized command %s\n", args.at(0).c_str() );
            continue;
        }
        
        calls.at(args.at(0))(args);
        device_calls::SendServerAlive(0x50);
        }
      
    return 0;
}

#endif	/* CMD_HPP */

