#ifndef CMD_HPP
#define	CMD_HPP
#include "api_calls.hpp"
#include "device.hpp"
#include <map>
#include <vector>
#include <string.h>


typedef int (*helper)(std::vector<string>);
typedef std::map<std::string,helper> call_map;

int HelpC(std::vector<string> args){
    //not implemented
    return 0;
}

int OpenDeviceC(std::vector<string> args){
    //not implemented
    return 0;
}

int DeviceInfoC(std::vector<string> args){
    //not implemented
    return 0;
}

int IdentC(std::vector<string> args){
    //not implemented
    return 0;
}

int ChannelAbleC(std::vector<string> args){
    // enable, disable, info 
    //not implemented
    return 0;
}

int PosCounterC(std::vector<string> args){
    // set, get
    //not implemented
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

int EOMC(std::vector<string> args){
    //enable, disable
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
    std::make_pair("eom", &EOMC),
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
        }
        
        if ( args.at(0).compare("exit") ) break;
        if ( calls.count(args.at(0))== 0 ) {
            printf("Unrecognized command %s\n", args.at(0).c_str() );
            continue;
        }
        
        int ret_val = calls.at(args.at(0))(args);
        if ( ret_val != 0 ) return ret_val;
    }
    
    return 0;
}

#endif	/* CMD_HPP */

