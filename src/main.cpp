#include <cstdlib>
#include <iostream>
#include "api_calls.hpp"
#include "init.hpp"
#include "log.hpp"
#include "../ftdi_lib/ftd2xx.h"
#include "../ftdi_lib/WinTypes.h"

using namespace std;

int UI = 1; // 1 -> cmd, 2-> graphical

int ParseCmdArgs(int argc, char** argv){
    return 0;
}
        
int main(int argc, char** argv) {
    ParseCmdArgs(argc, argv);
    int ret_code;
    ret_code = init();
    switch (ret_code){
        case STOP : {
            printf("Exiting\n");
            exit();
            return 0;
        }
        case SYSTEM_ERROR : {
            printf("Encountered system error. Exiting\n");
            LOG("System error\n")
            exit();
            return -1;
        }
        case FT_ERROR : {
            printf("Encountered error with FTDI library. Exiting\n");
            LOG("FTDI library error\n")
            exit();
            return -1;
        }
        case DEVICE_ERROR : {
            printf("Motor device encountered while initializing. Exiting\n");
            LOG("Device error\n")
            exit();
            return -1;
        }
        case FATAL_ERROR :{
            printf("Fatal error. Exiting\n");
            LOG("Other fatal error\n")
            exit();
            return -1;
        }
    }
    
    if (UI == 1) run_cmd();
    
    exit();
    return 0;
}

