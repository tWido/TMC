#ifndef CMD_HPP
#define	CMD_HPP
#include "api.hpp"
#include <unordered_map>
#include <vector>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

typedef int (*helper)(std::vector<string>);
typedef std::unordered_map<std::string,helper> call_map;

#define INVALID_CALL -50
#define ERR_CALL -51
#define UNSUPPORTED -52

#define HAS_FLAG(x) if ((opened_device.motor[at].status_status_bits & x) == x) 

#define GET_NUM(x) try {                                            \
                x = std::stoi(args.at(i+1), 0, 10);                 \
            }                                                       \
            catch(const std::exception& e) {                        \
                printf("Given argument is not a valid number\n");   \
                return INVALID_CALL;                                \
            }


#define CHANNEL_OPERATION(operation) if (args.size() <= i+1 ) {             \
                printf("Channel\\bay number not specified\n");              \
                return INVALID_CALL;                                        \
            }                                                               \
            GET_NUM(index)                                                  \
            if (opened_device.bays == -1){                                  \
                if (operation(0x50,index) == INVALID_CHANNEL ){             \
                    printf("Not existing channel number given\n");          \
                    return ERR_CALL;                                        \
                }                                                           \
            }                                                               \
            else {                                                          \
                index += 0x20;                                              \
                if (operation(index) == INVALID_DEST ){                     \
                    printf("Not existing channel number given\n");          \
                    return ERR_CALL;                                        \
                }                                                           \
            }                                                               \
            i++;

#define GET_MESSAGE(mess_type, call)                                \
        mess_type* mess = (mess_type*) malloc(sizeof(mess_type));   \
        if (opened_device.bays == -1){                              \
            if (call(mess, 0x50, index) == INVALID_CHANNEL){        \
                printf("Not existing channel number given\n");      \
                free(mess);                                         \
                return ERR_CALL;                                    \
            }                                                       \
        }                                                           \
        else {                                                      \
            index += 0x20;                                          \
            if (call(mess, index) == INVALID_DEST){                 \
                printf("Wrong address given\n");                    \
                free(mess);                                         \
                return ERR_CALL;                                    \
            }                                                       \
        }                                                                   


#define NULL_ARGS if (args.size() == 1) {       \
        printf("No arguments\n");               \
        return INVALID_CALL;                    \
    }


#define SET_FLAG if (args.at(i).compare("-s") == 0){                    \
            if (operation != -1) {                                      \
                printf("Operation has to be specified exactly once\n"); \
                return INVALID_CALL;                                    \
            }                                                           \
            operation = 1;                                              \
            if (args.size() <= i+1){                                    \
                printf("Not enough paramaters\n");                      \
                return INVALID_CALL;                                    \
            }                                                           \
            GET_NUM(index)                                              \
        }

#define GET_FLAG if (args.at(i).compare("-g") == 0){                    \
            if (operation != -1) {                                      \
                printf("Operation has to be specified exactly once");   \
                return INVALID_CALL;                                    \
            }                                                           \
            operation = 0;                                              \
            if (args.size() <= i+1){                                    \
                printf("Not enough parameters\n");                      \
                return INVALID_CALL;                                    \
            }                                                           \
            GET_NUM(index)                                              \
        }

#define FLAG(lookup_str, int_val, bool_val, err_string) \
            if (args.at(i).compare(lookup_str) == 0){   \
            if (args.size() <= i+1){                    \
                printf("Not enough parameters\n");      \
                return INVALID_CALL;                    \
            }                                           \
            if (bool_val){                              \
                printf(err_string);                     \
                return INVALID_CALL;                    \
            }                                           \
            GET_NUM(int_val)                            \
            bool_val = true;                            \
        }

#define HAS_FUNCTION(x) if (opened_device.functions.count(x) == 0) return UNSUPPORTED;

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
    printf(" homingvel   homing velocity \n");
    printf(" home        move to home position \n");
    printf(" relmove     start relative move \n");
    printf(" absmove     start absolute move \n");
    printf(" jogmove     start jog move \n");
    printf(" velmove     start move with set velocity \n");
    printf(" stop        stop movement \n");
    printf(" ledp        front LED parameters \n");
    printf(" buttp       device's buttons parameters \n");
    printf(" accp        acceleration profile\n");
    printf(" status      get motor status \n");
    printf(" swait       wait for stop\n");
    printf(" wait        wait for specified amount of seconds\n");
    return 0;
}

int OpenDeviceC(std::vector<string> args){
    NULL_ARGS
    if (args.size() > 3  || args.size() == 2) {
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
        int ret;
        ret = OpenDevice(num-1); //for user devices are numbered from 1
        if (ret == FT_ERROR ) return FT_ERROR;
        if (ret == INVALID_PARAM_1){
            printf("Incorrect device number\n");
            return ERR_CALL;
        }
        return 0;
    }
    if ( args.at(1).compare("-s") == 0) {
        if (args.size() == 2 ) {
            printf("Serial not specified\n");
            return INVALID_CALL;
        }
        int ret = 0;
        for (int i = 0; i< devices_connected; i++ ){
            std::string to_comp (connected_device[i].SN);
            if ( to_comp.compare(args.at(2)) == 0 ) ret = OpenDevice(i); 
        }
        if (ret == FT_ERROR ) return FT_ERROR;
        if (ret == INVALID_PARAM_1){
            printf("Incorrect device number\n");
            return ERR_CALL;
        }
        return 0;
    }
    printf("Unrecognized parameter %s, see -h for help\n", args.at(1).c_str());
    return 0;
}


int DeviceInfoC(std::vector<string> args){
    if (args.size() > 1 && args.at(1) == "-h"){ 
        printf("Prints information to all compatible devices connected\n");
        return 0;
    }
    if (args.size() > 1) printf("No arguments except -h\n");
    int prev_opened = opened_device_index;
    for (int i = 0; i< devices_connected; i++){
        if (OpenDevice(i) == FT_ERROR ) return FT_ERROR;
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
                }
            }
        }
        if (connected_device[i].channels != -1 ){ 
            printf(" Channels: %d\n", connected_device[i].channels);
            printf(" Channel 1\n");
            if (connected_device[i].channels == 2 ){
                printf(" Channel 2\n");
            }
        }
        HwInfo *info = (HwInfo*) malloc(sizeof(HwInfo));
        if (device_calls::GetHwInfo(info) != 0) printf("Error occured while receiving info from device\n");
        printf("  Hardware version: %d\n", info->HwVersion());
        printf("  Notes: %s\n", info->Notes().c_str());
        free(info);
    }
    
    if (OpenDevice(prev_opened) == FT_ERROR) return FT_ERROR;
    return 0;
}

int IdentC(std::vector<string> args){
    HAS_FUNCTION(IDENTIFY)
    if (args.size() == 1 ) {
        device_calls::Identify();
        return 0;
    }
    uint8_t dest = 0;

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
    if (device_calls::Identify(dest) == INVALID_DEST){ 
        printf("Invalid destination given\n");
        return ERR_CALL;
    }
    return 0;
}

int ChannelAbleC(std::vector<string> args){
    HAS_FUNCTION(GET_CHANENABLESTATE)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-e") == 0 || args.at(i).compare("-d") == 0 || args.at(i).compare("-i") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    for (unsigned int i = 1; i< args.size(); i++){
        uint8_t index = 0;
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
            GET_NUM(index)
            GetChannelState *state = (GetChannelState*) malloc(sizeof(GetChannelState));
            if (opened_device.bays == -1){     
                if (device_calls::ChannelState(state,0x50,index) == INVALID_CHANNEL){
                    printf("Not existing channel number given\n");
                    free(state);
                    return ERR_CALL;                                               
                }
                if (state->GetSecondParam() == 0x01 ) printf("Enabled\n");
                else printf("Disabled\n");
            }                                                               
            else {                                                          
                index += 0x20;                                                
                if (device_calls::ChannelState(state,index) == INVALID_DEST){
                    printf("Wrong address given\n");
                    free(state);
                    return ERR_CALL;  
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
    HAS_FUNCTION(GET_POSCOUNTER)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0) {i++; continue; }
        if (args.at(i).compare("-s") == 0) {i += 2; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("WARNING: Setting position counter isn't standard operation and may result in unexpected behavior\n");
            printf("Set or get actual position counter in device\n");
            printf("-s NUMBER VALUE      set position counter for given motor channel to specified VALUE\n");
            printf("-g NUMBER            get position counter for given motor channel\n");
        }
        if (args.at(i).compare("-s") == 0){
            if (args.size() <= i+2){ 
                printf("Not enough parameters\n");
                return INVALID_CALL;
            }
            uint8_t index = 0;
            int32_t value = 0;
            try {
                index = std::stoi(args.at(i+1), 0, 10);
                value = std::stoi(args.at(i+2), 0, 10);
            }
            catch(const std::exception& e) { 
                printf("Given argument is not a valid number\n");
                return INVALID_CALL;
            } 
            if (opened_device.bays == -1){
                if (device_calls::SetPositionCounter(value, 0x50, index) == INVALID_CHANNEL){
                    printf("Not existing channel number given\n");
                    return ERR_CALL;
                }
            }
            else {
                index += 0x20;
                if (device_calls::SetPositionCounter(value, index) == INVALID_DEST){
                    printf("Wrong address given\n");
                    return ERR_CALL;
                }
            }
        }
        
        if (args.at(i).compare("-g") == 0){
            if (args.size() <= i+i){ 
                printf("Not enough paramaters\n");
                return INVALID_CALL;
            }
            uint8_t index = 0;
            GET_NUM(index)
            GetPosCounter* counter = (GetPosCounter*) malloc(sizeof(GetPosCounter));
            if (opened_device.bays == -1){
                if (device_calls::GetPositionCounter(counter, 0x50, index) == INVALID_CHANNEL){
                    printf("Not existing channel number given\n");
                    free(counter);
                    return ERR_CALL;
                }
            }
            else {
                index += 0x20;
                if (device_calls::GetPositionCounter(counter, index) == INVALID_DEST){
                    printf("Wrong address given\n");
                    free(counter);
                    return ERR_CALL;
                }
            }
            printf("Position counter: %d\n", counter->GetPosition());
        }
    }
    return 0;
}

int EncCountC(std::vector<string> args){
    HAS_FUNCTION(GET_ENCCOUNTER)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0) {i++; continue; }
        if (args.at(i).compare("-s") == 0) {i += 2; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("WARNING: Setting encoder counter isn't standard operation and may result in unexpected behavior\n");
            printf("Set or get actual encoder counter in device\n");
            printf("-s NUMBER VALUE      set encoder counter for given motor channel to specified VALUE\n");
            printf("-g NUMBER            get encoder counter for given motor channel\n");
        }
        if (args.at(i).compare("-s") == 0){
            if (args.size() <= i+2){ 
                printf("Not enough paramaters\n");
                return INVALID_CALL;
            }
            uint8_t index = 0;
            int32_t value = 0;
            try {
                index = std::stoi(args.at(i+1), 0, 10);
                value = std::stoi(args.at(i+2), 0, 10);
            }
            catch(const std::exception& e) { 
                printf("Given argument is not a valid number\n");
                return INVALID_CALL;
            } 
            if (opened_device.bays == -1){
                if (device_calls::SetEncoderCounter(value, 0x50, index) == INVALID_CHANNEL){
                    printf("Not existing channel number given\n");
                    return ERR_CALL;
                }
            }
            else {
                index += 0x20;
                if (device_calls::SetEncoderCounter(value, index) == INVALID_DEST){
                    printf("Wrong address given\n");
                    return ERR_CALL;
                }
            }
        }
        
        if (args.at(i).compare("-g") == 0){
            if (args.size() <= i+i){ 
                printf("Not enough parameters\n");
                return INVALID_CALL;
            }
            uint8_t index = 0;
            GET_NUM(index)
            GetEncCount* counter = (GetEncCount*) malloc(sizeof(GetEncCount));
            if (opened_device.bays == -1){
                if (device_calls::GetEncoderCounter(counter, 0x50, index) == INVALID_CHANNEL){
                    printf("Not existing channel number given\n");
                    free(counter);
                    return ERR_CALL;
                }
            }
            else {
                index += 0x20;
                if (device_calls::GetEncoderCounter(counter, index) == INVALID_DEST){
                    printf("Wrong address given\n");
                    free(counter);
                    return ERR_CALL;
                }
            }
            printf("Encoder counter: %d\n", counter->GetEncCounter());
        }
    }
    return 0;
}

int VelParamC(std::vector<string> args){
    HAS_FUNCTION(GET_VELPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-m") == 0 || args.at(i).compare("-a") == 0) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    int32_t acc = 0;
    int32_t maxvel = 0;
    bool acc_spec = false;
    bool maxvel_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get velocity parameters for specified motor channel. Parameters are acceleration and maximum velocity.\n");
            printf("Velocity is specified in encoder counts per second or microsteps per second, depending on controller. Acceleration is specified in counts/sec/sec or microsteps/sec/sec. \n");
            printf("-s NUMBER       set operation for given motor channel, setting both parameters is mandatory\n");
            printf("-g NUMBER       get parameters for given motor channel\n");
            printf("-m VALUE        set maximum velocity\n");
            printf("-a VALUE        acceleration\n");
            return 0;
        }
        
        SET_FLAG   
        GET_FLAG
        FLAG("-m", maxvel, maxvel_spec, "Maximum velocity already specified\n")
        FLAG("-a", acc, acc_spec, "Acceleration already specified\n")
        
    }
    
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetVelocityParams, device_calls::GetVelocityP)
        printf("Acceleration: %d\n",mess->GetAcceleration());
        printf("Maximum velocity: %d\n",mess->GetMaxVel());
    }
    if (operation == 1){
        int ret;
        if(!acc_spec || !maxvel_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetVelocityP(acc, maxvel, 0x50, index);
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Velocity given exceeds maximal velocity\n"); return ERR_CALL;}
                case INVALID_PARAM_2: {printf("Acceleration given exceed maximal acceleration\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetVelocityP(acc, maxvel, index);
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Velocity given exceeds maximal velocity\n"); return ERR_CALL;}
                case INVALID_PARAM_2: {printf("Acceleration given exceed maximal acceleration\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int JogParamC(std::vector<string> args){
    HAS_FUNCTION(GET_JOGPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-t") == 0 || args.at(i).compare("-m") == 0
                || args.at(i).compare("-a") == 0 || args.at(i).compare("-z") == 0 || args.at(i).compare("-v") == 0) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    int32_t maxvel = 0, acc = 0, step_size = 0;
    uint16_t mode = 0, stop_mode = 0;
    bool acc_spec = false, maxvel_spec = false, step_size_spec = false, mode_spec = false, stop_mode_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get jog parameters\n");
            printf("-s NUMBER       set operation for given motor channel, setting all parameters is mandatory\n");
            printf("-g NUMBER       get parameters for given motor channel\n");
            printf("-m VALUE        mode: 1 for continuous jogging, 2 for single step\n");
            printf("-v VALUE        maximal velocity\n");
            printf("-a VALUE        acceleration\n");
            printf("-z VALUE        step size\n");
            printf("-t VALUE        stop mode: 1 for immediate stop, 2 for profiled\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-m", mode, mode_spec, "Mode already specified\n")
        FLAG("-v", maxvel, maxvel_spec, "Maximal velocity already specified\n")
        FLAG("-a", acc, acc_spec, "Acceleration already specified\n")
        FLAG("-z", step_size, step_size_spec, "Step size already specified\n")
        FLAG("-t", stop_mode, stop_mode_spec, "Stop mode already specified\n")
    }
    
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetJogParams, device_calls::GetJogP)
        printf("Acceleration: %d\n",mess->GetAcceleration());
        printf("Maximum velocity: %d\n",mess->GetMaxVel());
        printf("Mode: %d\n",mess->GetJogMode());
        printf("Stop mode: %d\n",mess->GetStopMode());
        printf("Step size: %d\n",mess->GetStepSize());
    }
    if (operation == 1){
        int ret = 0;
        if(!acc_spec || !maxvel_spec || !step_size_spec || !mode_spec || !stop_mode_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetJogP(mode, step_size, maxvel, acc, stop_mode, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Invalid mode given\n"); return ERR_CALL;}
                case INVALID_PARAM_3: {printf("Velocity given exceeds maximal velocity\n"); return ERR_CALL;}
                case INVALID_PARAM_4: {printf("Acceleration given exceed maximal acceleration\n"); return ERR_CALL;}
                case INVALID_PARAM_5: {printf("Invalid stop mode given\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetJogP(mode, step_size, maxvel, acc, stop_mode, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Invalid mode given\n"); return ERR_CALL;}
                case INVALID_PARAM_3: {printf("Velocity given exceeds maximal velocity\n"); return ERR_CALL;}
                case INVALID_PARAM_4: {printf("Acceleration given exceed maximal acceleration\n"); return ERR_CALL;}
                case INVALID_PARAM_5: {printf("Invalid stop mode given\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int PowerParamC(std::vector<string> args){
    HAS_FUNCTION(GET_POWERPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-r") == 0  || args.at(i).compare("-m") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    uint16_t mfact = 0, rfact = 0;
    bool mfact_spec = false, rfact_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get power used when motor is moving and resting\n");
            printf("-s NUMBER       set power parameters\n");
            printf("-g NUMBER       get power parameters\n");
            printf("-m VALUE        move factor in %%\n");
            printf("-r VALUE        rest factor in %%\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-m", mfact, mfact_spec, "Move power factor already specified\n")
        FLAG("-r", rfact, rfact_spec, "Rest power factor already specified\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetPowerParams, device_calls::GetPowerUsed)
        printf("Rest factor: %d\n",mess->GetRestFactor());
        printf("Move factor: %d\n",mess->GetMoveFactor());
    }
    if (operation == 1){
        int ret;
        if(!mfact_spec || !rfact ) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetPowerUsed(rfact, mfact, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Invalid rest factor given\n"); return ERR_CALL;}
                case INVALID_PARAM_2: {printf("Invalid move factor given\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetPowerUsed(rfact, mfact, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Invalid rest factor given\n"); return ERR_CALL;}
                case INVALID_PARAM_2: {printf("Invalid move factor given\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int BacklashDistC(std::vector<string> args){
    HAS_FUNCTION(GET_GENMOVEPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-d") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    int32_t dist = 0;
    bool dist_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get backlash distance\n");
            printf("-s NUMBER       set distance parameters\n");
            printf("-g NUMBER       get distance parameters\n");
            printf("-d VALUE        value of backlash distance\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-d", dist, dist_spec, "Distance already specified\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetGeneralMoveParams, device_calls::GetBacklashDist)
        printf("Backlash distance: %d\n",mess->GetBacklashDist());
    }
    if (operation == 1){
        int ret = 0;
        if(!dist_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetBacklashDist(dist, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetBacklashDist(dist, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int RelMoveParamC(std::vector<string> args){
    HAS_FUNCTION(GET_MOVERELPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-d") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    int32_t dist = 0;
    bool dist_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get relative move distance, value is used in next call for relative move\n");
            printf("-s NUMBER       set relative move parameters\n");
            printf("-g NUMBER       get relative move parameters\n");
            printf("-d VALUE        distance to move\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-d", dist, dist_spec, "Distance already specified\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetRelativeMoveParams, device_calls::GetRelativeMoveP)
        printf("Relative distance held in controller: %d\n",mess->GetRelativeDist());
    }
    if (operation == 1){
        int ret;
        if(!dist_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetRelativeMoveP(dist, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetRelativeMoveP(dist, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int AbsMoveParamC(std::vector<string> args){
    HAS_FUNCTION(GET_MOVEABSPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-p") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    int32_t pos = 0;
    bool pos_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get absolute move distance, value is used in next call for absolute move\n");
            printf("-s NUMBER       set absolute move parameters\n");
            printf("-g NUMBER       get absolute parameters\n");
            printf("-p VALUE        position to move to\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-p", pos, pos_spec, "Position already specified\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetAbsoluteMoveParams, device_calls::GetAbsoluteMoveP)
        printf("Absolute position for next move held in controller: %d\n",mess->GetAbsolutePos());
    }
    if (operation == 1){
        int ret = 0;
        if(!pos_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetAbsoluteMoveP(pos, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetAbsoluteMoveP(pos, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int HomingVelC(std::vector<string> args){
    HAS_FUNCTION(GET_HOMEPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-v") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    int32_t vel = 0;
    bool vel_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get homing velocity\n");
            printf("-s NUMBER       set homing velocity\n");
            printf("-g NUMBER       get homing velocity\n");
            printf("-v VALUE        velocity\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-v", vel, vel_spec, "Velocity already specified\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetHomeParams, device_calls::GetHomingVel)
        printf("Homing velocity: %d\n",mess->GetHomingVelocity());
    }
    if (operation == 1){
        int ret = 0;
        if(!vel_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetHomingVel(vel, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Velocity given exceeds maximum velocity\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetHomingVel(vel, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Velocity given exceeds maximum velocity\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int HomeC(std::vector<string> args){
    HAS_FUNCTION(MOVE_HOME)
    if (args.size() == 1){ 
        device_calls::MoveToHome();
        return 0;
    }
    if (args.at(1).compare("-h") == 0) {
        printf("Start move to home position\n");
        printf("-i NUMBER           index of motor to start homing, default is 1\n");
        return 0;
    }
    else if (args.at(1).compare("-i") == 0){
        if (args.size() == 2) {
            printf("Motor index not specified\n");
            return INVALID_CALL;
        }
        uint8_t index = 1;
        int i = 1;
        GET_NUM(index)
        if (opened_device.bays == -1){
           if( device_calls::MoveToHome(0x50, index) == INVALID_CHANNEL ){
               printf("Not existing channel given\n");
               return ERR_CALL;
           } 
        }
        else {
            index += 0x20;
            if ( device_calls::MoveToHome(index) == INVALID_DEST){
                printf("Wrong address given\n"); 
                return ERR_CALL;
            }; 
        }       
    }
    else {
        printf("Unknown argument %s\n",args.at(1).c_str());
        return INVALID_CALL;
    }
    return 0;
}

int StartRelMoveC(std::vector<string> args){
    HAS_FUNCTION(MOVE_RELATIVE)
    if (args.size() == 1){ 
        device_calls::StartSetRelativeMove();
        return 0;
    }
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-d") == 0 || args.at(i).compare("-i") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
            printf("Unknown parameter %s\n",args.at(i).c_str());
            return INVALID_CALL;
        }
    }
    uint8_t index = 1;
    int32_t dist = 0;
    bool dist_spec = false, index_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Start relative move. Distance can be either specified or previously set by relmovep command.\n");
            printf("-i NUMBER           index of motor to start move, default is 1\n");
            printf("-d VALUE            distance to move, if not specified parameter stored in device is used");
            return 0;
        }
        
        FLAG("-i", index, index_spec, "Index set more than once\n")
        FLAG("-d", dist, dist_spec, "Distance set more than once\n")
    }

    int ret;
    if (opened_device.bays == -1){
        if (dist_spec) ret = device_calls::StartSetRelativeMove(0x50, index);
        else ret = device_calls::StartRelativeMove(dist, 0x50, index); 
        switch (ret){
            case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
        };
    }
    else {
        index += 0x20;
        if (dist_spec) ret = device_calls::StartSetRelativeMove(index);
        else ret = device_calls::StartRelativeMove(dist, index); 
        switch (ret){
            case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
        };
    }
    return 0;
}

int StartAbsMoveC(std::vector<string> args){
    HAS_FUNCTION(MOVE_ABSOLUTE)
    if (args.size() == 1){ 
        device_calls::StartSetAbsoluteMove();
        return 0;
    }
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-p") == 0 || args.at(i).compare("-i") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
            printf("Unknown parameter %s\n",args.at(i).c_str());
            return INVALID_CALL;
        }
    }
    uint8_t index = 1;
    int32_t pos = 0;
    bool pos_spec = false, index_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Start absolute move. Position can be either specified or previously set by absmovep command.\n");
            printf("-i NUMBER           index of motor to start move, default is 1\n");
            printf("-p VALUE            position to move to, if not specified parameter stored in device is used");
            return 0;
        }
        
        FLAG("-i", index, index_spec, "Index set more than once\n")
        FLAG("-p", pos, pos_spec, "Position set more than once\n")
    }

    int ret = 0;
    if (opened_device.bays == -1){
        if (pos_spec) ret = device_calls::StartSetAbsoluteMove(0x50, index);
        else ret = device_calls::StartAbsoluteMove(pos, 0x50, index); 
        switch (ret){
            case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Position given exceeds maximal position\n"); return ERR_CALL;}
        };
    }
    else {
        index += 0x20;
        if (pos_spec) ret = device_calls::StartSetAbsoluteMove(index);
        else ret = device_calls::StartAbsoluteMove(pos, index); 
        switch (ret){
            case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Position given exceeds maximal position\n"); return ERR_CALL;}
        };
    }
    return 0;
}

int StartJogMoveC(std::vector<string> args){
    HAS_FUNCTION(MOVE_JOG)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-d") == 0 || args.at(i).compare("-i") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
            printf("Unknown parameter %s\n",args.at(i).c_str());
            return INVALID_CALL;
        }
    }
    uint8_t index = 1;
    uint8_t direction = 0;
    bool direction_spec = false, index_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Start jog move. Jog move parameters can be specified by jogmovep call. Setting direction is mandatory.\n");
            printf("-i NUMBER           index of motor to start move, default is 1\n");
            printf("-d VALUE            direction: 1 -> forward, 2 -> reverse, mandatory argument\n");
            return 0;
        }
        
        FLAG("-i", index, index_spec, "Index set more than once\n")
        FLAG("-d", direction, direction_spec, "Direction set more than once\n")
    }

    int ret;
    if(!direction_spec) {
        printf("Not all mandatory parameters specified\n");
        return INVALID_CALL;
    }
    if (opened_device.bays == -1){
        ret = device_calls::StartJogMove(direction, 0x50, index); 
        switch (ret){
            case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Not existing direction parameter given\n"); return ERR_CALL;}
        };
    }
    else {
        index += 0x20;
        ret = device_calls::StartJogMove(direction, index); 
        switch (ret){
            case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Not existing direction parameter given\n"); return ERR_CALL;}
        };
    }
    return 0;
}

int StartVelMoveC(std::vector<string> args){
    HAS_FUNCTION(MOVE_VELOCITY)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-d") == 0 || args.at(i).compare("-i") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
            printf("Unknown parameter %s\n",args.at(i).c_str());
            return INVALID_CALL;
        }
    }
    uint8_t index = 1;
    uint8_t direction = 0;
    bool direction_spec = false, index_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Start move in specified direction. Velocity parameters can be set with velp call. Setting direction is mandatory.\n");
            printf("-i NUMBER           index of motor to start move, default is 1\n");
            printf("-d VALUE            direction: 1 -> forward, 2 -> reverse, mandatory argument ");
            return 0;
        }
        
        FLAG("-i", index, index_spec, "Index set more than once\n")
        FLAG("-d", direction, direction_spec, "Direction set more than once\n")
    }

    int ret = 0;
    if(!direction_spec) {
        printf("Not all mandatory parameters specified\n");
        return INVALID_CALL;
    }
    if (opened_device.bays == -1){
        ret = device_calls::StartJogMove(direction, 0x50, index); 
        switch (ret){
            case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Not existing direction parameter given\n"); return ERR_CALL;}
        };
    }
    else {
        index += 0x20;
        ret = device_calls::StartJogMove(direction, index); 
        switch (ret){
            case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Not existing direction parameter given\n"); return ERR_CALL;}
        };
    }
    return 0;
}

int StopC(std::vector<string> args){
    HAS_FUNCTION(MOVE_STOP)
    if (args.size() == 1){ 
        device_calls::StopMovement();
        return 0;
    }
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-m") == 0 || args.at(i).compare("-i") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
            printf("Unknown parameter %s\n",args.at(i).c_str());
            return INVALID_CALL;
        }
    }
    uint8_t index = 1;
    uint8_t stop_mode = 2;
    bool mode_spec = false, index_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Stops any move on specified motor\n");
            printf("-i NUMBER       motor index, optional argument, default is 1\n");
            printf("-m NUMBER       stop mode: 1 -> immediate stop, 2 -> profiled stop, optional argument, default is 2\n");
            return 0;
        }
        
        FLAG("-i", index, index_spec, "Index set more than once\n")
        FLAG("-m", stop_mode, mode_spec, "Stop mode set more than once\n")
    }

    int ret;
    if (opened_device.bays == -1){
        ret = device_calls::StopMovement(stop_mode, 0x50, index); 
        switch (ret){
            case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Invalid stop mode\n"); return ERR_CALL;}
        };
    }
    else {
        index += 0x20;
        ret = device_calls::StopMovement(stop_mode, index); 
        switch (ret){
            case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
            case INVALID_PARAM_1: {printf("Invalid stop mode\n"); return ERR_CALL;}
        };
    }
    
    return 0;
}

int AccParamC(std::vector<string> args){
    HAS_FUNCTION(GET_BOWINDEX)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-d") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    uint16_t accp = 0;
    bool accp_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get acceleration profile used by controller.\n");
            printf("-s NUMBER       set acceleration profile\n");
            printf("-g NUMBER       get acceleration profile\n");
            printf("-p NUMBER       acceleration profile: range from 0 to 18, 0 for trapezoidal profile, 1-18 are S-curve profiles\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-p", accp, accp_spec, "Acceleration profile already set\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetBowIndex, device_calls::GetAccelerationProfile)
        printf("Acceleration profile: %d\n",mess->BowIndex());
    }
    if (operation == 1){
        int ret;
        if(!accp_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetAccelerationProfile(accp, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Not existing profile number\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetAccelerationProfile(accp, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Not existing profile number\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int LedParamC(std::vector<string> args){
    HAS_FUNCTION(GET_AVMODES)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-m") == 0 ) {i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    uint16_t mode = 0;
    bool mode_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get LED parameter used by controller.\n");
            printf("-s NUMBER       set LED parameters\n");
            printf("-g NUMBER       get LED parameters\n");
            printf("-m NUMBER       mode: 1 -> LED flashes on call, 2 -> LED flashes when limit switch is reached, 8 -> lit while moving \n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-m", mode, mode_spec, "Mode already set\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetLedMode, device_calls::GetLedP)
        printf("Mode actually set: %d\n",mess->GetMode());
    }
    if (operation == 1){
        int ret;
        if(!mode_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetLedP(mode, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Not existing mode number\n"); return ERR_CALL;}
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetLedP(mode, index); 
            switch (ret){
                case INVALID_DEST:  {printf("Wrong address given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Not existing mode number\n"); return ERR_CALL;}
            };
        }
    }
    return 0;
}

int ButtonsParamC(std::vector<string> args){
    HAS_FUNCTION(GET_BUTTONPARAMS)
    NULL_ARGS
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-g") == 0 || args.at(i).compare("-s") == 0 || args.at(i).compare("-m") == 0 || args.at(i).compare("-p") == 0  
                || args.at(i).compare("-P") == 0 || args.at(i).compare("-t") == 0 ){ i++; continue; }
        if (args.at(i).compare("-h") != 0 ){
                printf("Unknown parameter %s\n",args.at(i).c_str());
                return INVALID_CALL;
            }
    }
    int operation = -1; // -1 unspecified, 0 get, 1 set
    uint8_t index = 0;
    uint16_t mode = 0, timeout = 0;
    int32_t pos1 = 0, pos2 = 0;
    bool mode_spec = false, timeout_spec = false, pos1_spec = false, pos2_spec = false;
    for (unsigned int i = 1; i< args.size(); i++){
        if (args.at(i).compare("-h") == 0){
            printf("Set or get button parameters. When creating new setting, all parameters has to be specified.\n");
            printf("-s NUMBER       set new buttons configuration\n");
            printf("-g NUMBER       get current buttons configuration\n");
            printf("-m NUMBER       mode: 1 -> jog move (parameters can be set by jogp), 2 -> move to position specified by -p(button 1) and -P(button 2)\n");
            printf("-p VALUE        position to move to, when button 1 is pressed\n");
            printf("-P VALUE        position to move to, when button 2 is pressed \n");
            printf("-t VALUE        specifies time in ms that the button must be depressed\n");
            return 0;
        }
        
        SET_FLAG
        GET_FLAG
        FLAG("-m", mode, mode_spec, "Mode already set\n")
        FLAG("-p", pos1, pos1_spec, "Position 1 already set\n")
        FLAG("-P", pos2, pos2_spec, "Position 2 already set\n")
        FLAG("-t", timeout, timeout_spec, "Timeout already set\n")
    }
    if (operation == -1) {
        printf("Operation not specified\n");
        return INVALID_CALL;
    }
    if (operation == 0){
        GET_MESSAGE(GetButtonParams, device_calls::GetButtonsInfo)
        printf("Mode actually set: %d\n", mess->GetMode());
        printf("Position 1: %d\n", mess->GetPosition1());
        printf("Position 2: %d\n", mess->GetPosition2());
        printf("Timeout: %d\n", mess->GetTimeout());
    }
    if (operation == 1){
        int ret;
        if(!mode_spec || !pos1_spec || !pos2_spec || !timeout_spec) {
            printf("Not all mandatory parameters specified\n");
            return INVALID_CALL;
        }
        if (opened_device.bays == -1){
            ret = device_calls::SetButtons(mode, pos1, pos2, timeout, 0x50, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Not existing mode number\n"); return ERR_CALL;}
                case INVALID_PARAM_2: {printf("Position 1 exceeds maximal position\n"); return ERR_CALL;}
                case INVALID_PARAM_3: {printf("Position 2 exceeds maximal position\n"); return ERR_CALL;}
                case IGNORED_PARAM: {printf("NOTE: Timeout is ignored in this controller device\n"); }
            };
        }
        else {
            index += 0x20;
            ret = device_calls::SetButtons(mode, pos1, pos2, timeout, index); 
            switch (ret){
                case INVALID_CHANNEL: {printf("Not existing channel given\n"); return ERR_CALL;}
                case INVALID_PARAM_1: {printf("Not existing mode number\n"); return ERR_CALL;}
                case INVALID_PARAM_2: {printf("Position 1 exceeds maximal position\n"); return ERR_CALL;}
                case INVALID_PARAM_3: {printf("Position 2 exceeds maximal position\n"); return ERR_CALL;}
                case IGNORED_PARAM: {printf("NOTE: Timeout is ignored in this controller device\n"); }
            };
        }
    }
    return 0;
}

void PrintStatus(int at){
    printf("Position: %d\n", opened_device.motor[at].status_position);
    if (opened_device.functions.count(GET_STATUSUPDATE) == 1) printf("Encoder count: %d\n", opened_device.motor[at].status_enc_count);
    if (opened_device.functions.count(GET_DCSTATUSUPDATE) == 1) printf("Velocity: %d\n", opened_device.motor[at].status_velocity);
    
    HAS_FLAG(0x00000001) printf("Forward hardware switch active\n");
    HAS_FLAG(0x00000002) printf("Reverse hardware switch active\n");
    HAS_FLAG(0x00000004) printf("Forward software switch active\n");
    HAS_FLAG(0x00000008) printf("Reverse software switch active\n");
    HAS_FLAG(0x00000010) printf("Moving forward\n");
    HAS_FLAG(0x00000020) printf("Moving reverse\n");
    HAS_FLAG(0x00000040) printf("Jogging forward\n");
    HAS_FLAG(0x00000080) printf("Jogging reverse\n");
    HAS_FLAG(0x00000100) printf("Motor connected\n");
    HAS_FLAG(0x00000200) printf("Homing\n");
    HAS_FLAG(0x00000400) printf("Homed\n");
    HAS_FLAG(0x00001000) printf("Tracking\n");
    HAS_FLAG(0x00002000) printf("Settled\n");
    HAS_FLAG(0x00004000) printf("Motion error\n");
    HAS_FLAG(0x01000000) printf("Motor limit reached\n");
    HAS_FLAG(0x80000000) printf("Channel enabled\n");
}

int StatusC(std::vector<string> args){
    uint8_t index = 0;
    NULL_ARGS
    if (args.at(1).compare("-h") == 0){
        printf("Prints info from status message.\n");
        printf("-i NUMBER           index of motor to get info from\n");
        return 0;
    }
    else  if (args.at(1).compare("-i") == 0){
        int i = 1;
        GET_NUM(index);
        if (opened_device.bays == -1){
            if ( opened_device.channels < index ){
                printf("Not existing motor index given\n");
                return ERR_CALL;
            }
            PrintStatus(index-1);
        }
        else {
            if (opened_device.bays < index || opened_device.bay_used[index-1] == false){
                printf("Not existing motor index given\n");
                return ERR_CALL;
            }
            PrintStatus(index-1);
        }
    }
    else {
        printf("Unknown parameter\n");
        return INVALID_CALL;
    }
    return 0;
}

int WaitC(std::vector<string> args){
    NULL_ARGS
    if (args.at(1).compare("-h") == 0){
        printf("Blocks all commands for specified amount of time\n");
        printf("-s NUMBER       seconds to wait, mandatory\n");
        return 0;
    } else if (args.at(1).compare("-s") == 0){
        if (args.size() == 2){ 
            printf("Time not specified\n");
            return INVALID_CALL;
        }
        int seconds = 0;
        int i = 1;
        GET_NUM(seconds);
        if (seconds < 0 || seconds > INT_MAX) {
            printf("Invalid number of seconds\n");
            return INVALID_CALL;
        }
        for (int i = 0; i < seconds; i++){
            EmptyIncomingQueue();
            sleep(1);
        }
    }
    else {
        printf("Unknown argument\n");
        return INVALID_CALL;
    }
    return 0;
}

bool end_wait;

void StopWait(int sig_num){
    if (sig_num == SIGTSTP) end_wait = true;
}

int WaitForStopC(std::vector<string> args){
    NULL_ARGS
    if (args.at(1).compare("-h") == 0){
        printf("Blocks any command until motor is in stable position\n");
        printf("-i        motor index, mandatory\n");
        return 0;
    }
    else if (args.at(1).compare("-i") == 0){
        if (args.size() == 2){ 
            printf("Motor index not specified\n");
            return INVALID_CALL;
        }
        int i = 1;
        uint8_t index = 0;
        GET_NUM(index);
        end_wait = false;
        if (signal(SIGTSTP, &StopWait) == SIG_ERR)fprintf(stderr,"Failed to use signal handler, wait cannot be interrupted\n");
        else printf("Started wait, press ctrl+z to end wait\n");
        while(true){
            GET_MESSAGE(GetStatusBits, device_calls::GetStatBits)
            opened_device.motor[mess->GetMotorID()].status_status_bits = mess->GetStatBits();
            if (end_wait) break;
            if ((mess->GetStatBits() & 0x00000010) == 0x00000010 || (mess->GetStatBits() & 0x00000020) == 0x00000020 || (mess->GetStatBits() & 0x00000040) == 0x00000040 || 
                    (mess->GetStatBits() & 0x00000080) == 0x00000080 || (mess->GetStatBits() & 0x00000200) == 0x00000200 ) sleep(1);
            else break;
        }
        signal(SIGTSTP, SIG_DFL);
        printf("Motor in stable position\n");
    }
    else {
        printf("Unknown argument\n");
        return INVALID_CALL;
    }    
    return 0;
}

int reserve_fd;
void RedirectOutput(){
    reserve_fd = dup(STDOUT_FILENO);
    FILE *void_file = fopen("/dev/null","w");
    if (void_file == NULL) printf("Failed to open dev/null, undefined output\n");
    dup2(fileno(void_file), STDERR_FILENO);
    dup2(fileno(void_file), STDOUT_FILENO);
    fclose(void_file);
    return;
}

void EnableOutput(){
    fflush(stdout);
    dup2(reserve_fd, STDOUT_FILENO);
    close(reserve_fd);
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
    std::make_pair("swait", &WaitForStopC),
    std::make_pair("wait", &WaitC)
};


int run_cmd(int mode){
    fd_set in_set;
    struct timeval time;
    int ret;
    int command_num = 1;
    if (mode == 3 ) {
        RedirectOutput();
        opened_device.functions = all_set;
    }
    if (mode != 3) printf("Awaiting commands. Type \"help\" to display available commands.\n");
    //--------------------------- main loop ------------------------------------
    unsigned int tick = 0;
    while(true){
        FD_ZERO(&in_set);
        FD_SET(STDIN_FILENO, &in_set);
        time.tv_sec = 1;
        time.tv_usec = 0;
        //empty input buffer every second if no command occurs
        ret = select(1, &in_set, NULL, NULL, &time);
        if (ret == -1) return FATAL_ERROR;
        if (ret == 0 ){
            tick++;
            EmptyIncomingQueue();
            if (mode != 3 && (tick % 5 == 0))  device_calls::SendServerAlive(0x50);
            continue;
        }
        //command given, parse arguments
        
        std::string line = "";
        std::getline(std::cin, line);
        if (line == "" ) {command_num++ ;continue;}
        
        std::vector<std::string> args;
        string tok; 
        stringstream ss(line);
        while (ss >> tok) args.push_back(tok);  
        
        if ( args.at(0).compare("exit") == 0 ) break;
        
        if ( calls.count(args.at(0))== 0 ) {         //unrecognized command
            if (mode == 3) { 
                EnableOutput();
                printf("File not valid on line %d, unrecognized command\n", command_num);
                return 0;
            } 
            printf("Unrecognized command %s\n", args.at(0).c_str() );
            continue;
        }
        
        ret = calls.at(args.at(0))(args);           //call function from call_map
        if (ret == UNSUPPORTED) printf("Command not used in controller\n"); 
        if (ret == FT_ERROR && mode != 3) return FT_ERROR;
        if (ret == INVALID_CALL) {
            if (mode == 3) {
                EnableOutput();
                printf("File not valid on line %d, bad command syntax\n", command_num);
                return 0;
            }
            else printf("Invalid syntax\n");
        }
        
        if (mode != 3 && (command_num % 5 == 0)) device_calls::SendServerAlive(0x50);
        command_num++;
        }
    
    if (mode == 3) {
        EnableOutput();
        printf("File validated\n");
    }  
    return 0;
}

#endif	/* CMD_HPP */

