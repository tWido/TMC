#include <cstdlib>
#include <iostream>
#include "api_calls.hpp"
#include "init.hpp"
#include "cmd.hpp"
#include "../ftdi_lib/ftd2xx.h"
#include "../ftdi_lib/WinTypes.h"

using namespace std;

int UI = 1; // 1 -> cmd, 2-> graphical

int ParseCmdArgs(int argc, char** argv){
    return 0;
}
        
void FailExit(int ret_code){

switch (ret_code){
        case STOP : {
            exit();
        }
        case SYSTEM_ERROR : {
            printf("Encountered system error.\n");
            exit();
        }
        case FT_ERROR : {
            printf("Encountered error with FTDI library.\n");
            exit();
        }
        case DEVICE_ERROR : {
            printf("Motor device encountered error.\n");
            exit();
        }
        case FATAL_ERROR :{
            printf("Fatal error.\n");
            exit();
        }
    }

}

int main(int argc, char** argv) {
    ParseCmdArgs(argc, argv);
    int ret_code = 0;
    ret_code = init();
    if ( ret_code != 0 ){ 
        FailExit(ret_code);
        return ret_code;
    }
    
    if (UI == 1) ret_code = run_cmd();
    if ( ret_code != 0 ) {
        FailExit(ret_code);
        return ret_code;
    }
    
    exit();
    return 0;
}

