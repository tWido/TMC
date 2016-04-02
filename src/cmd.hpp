#ifndef CMD_HPP
#define	CMD_HPP
#include "api_calls.hpp"
#include "device.hpp"
#include <map>
#include <vector>


typedef int (*helper)(std::vector<string>);
typedef std::map<std::string,helper> call_map;

int IdentC(std::vector<string> args){
    //not implemented
    return 0;
}

int EnableChanC(std::vector<string> args){
    //not implemented
    return 0;
}

call_map calls = {
    std::make_pair("Flash", &IdentC),
    std::make_pair("ChanEn", &EnableChanC)
};


int run_cmd(){
   
    return 0;
}

#endif	/* CMD_HPP */

