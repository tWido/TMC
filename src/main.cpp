#include <cstdlib>
#include <iostream>
#include "api_calls.hpp"
#include "init.hpp"
#include "../ftdi_lib/ftd2xx.h"
#include "../ftdi_lib/WinTypes.h"

using namespace std;

int main(int argc, char** argv) {
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
            exit();
            return -1;
        }
        case FT_ERROR : {
            printf("Encountered error with FTDI library. Exiting\n");
            exit();
            return -1;
        }
        case DEVICE_ERROR : {
            printf("Motor device encountered while initializing. Exiting\n");
            exit();
            return -1;
        }
        case FATAL_ERROR :{
            printf("Fatal error. Exiting\n");
            exit();
            return -1;
        }
    }
    
    
    exit();
    return 0;
}

