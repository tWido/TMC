#include <cstdlib>
#include <iostream>
#include "api_calls.hpp"
#include "init.hpp"
#include "cmd.hpp"
#include "../ftdi_lib/ftd2xx.h"
#include "../ftdi_lib/WinTypes.h"

using namespace std;

int UI = 1; // 1 -> cmd, 2-> graphical, 3 - > validate file

int ParseCmdArgs(int argc, char** argv){
    return 0;
}
        
void FailExit(int ret_code){

switch (ret_code){
        case STOP : {
            exit();
            break;
        }
        case SYSTEM_ERROR : {
            printf("Encountered system error.\n");
            exit();
            break;
        }
        case FT_ERROR : {
            printf("Encountered error with FTDI library.\n");
            exit();
            break;
        }
        case DEVICE_ERROR : {
            printf("Motor device encountered error.\n");
            exit();
            break;
        }
        case FATAL_ERROR :{
            printf("Fatal error.\n");
            exit();
            break;
        }
    }

}

void RedirectOutput(){
    FILE *void_file = fopen("/dev/null","w");
    if (void_file == NULL) printf("Failed to open dev/null, undefined output\n");
    dup2(fileno(void_file), STDOUT_FILENO);
    fclose(void_file);
    return;
}

int main(int argc, char** argv) {
    ParseCmdArgs(argc, argv);
    int ret_code = 0;
    if (UI !=3) ret_code = init();
    if ( ret_code != 0 ){ 
        FailExit(ret_code);
        return ret_code;
    }
    
    if (UI == 1) ret_code = run_cmd(1);
    if (UI == 3) {
        RedirectOutput();
        ret_code = run_cmd(3);
    }
    if ( ret_code != 0 ) {
        FailExit(ret_code);
        return ret_code;
    }
    
    exit();
    return 0;
}

