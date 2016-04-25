#include <cstdlib>
#include <iostream>
#include "init.hpp"
#include "cmd.hpp"
#include "gui.hpp"
#include "../ftdi_lib/ftd2xx.h"
#include "../ftdi_lib/WinTypes.h"

using namespace std;

int UI = 1; // 1 -> cmd, 2-> graphical, 3 - > validate file
        
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
unsigned int devices_connected ;
controller_device *connected_device;
controller_device opened_device ;
unsigned int opened_device_index ;

 int run_gui(){
    char **argv = NULL;
    int argc = 0;
    QApplication a( argc, argv );
    GUI *gui = new GUI();
    gui->Setup();
    
    a.exec();
    return 0;
}


int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0) {
            printf("Options\n");
            printf("  -h, --help                    prints this help message and exit\n");
            printf("  -c, --command-line            starts with command line interface, default\n");
            printf("  -g, --graphical               after initialization starts graphical interface\n");
            printf("  -v, --validate                validate syntax of commands; use with \"< FILE\" to validate control file\n");
            return 0;
        }
        else if (strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--command-line") == 0) UI=1;
        else if (strcmp(argv[i],"-g") == 0 || strcmp(argv[i],"--graphical") == 0) UI=2;
        else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"--validate-file") == 0) UI=3;
        else {
            printf("Invalid option: %s\n", argv[i]);
            printf("Options\n");
            printf("  -h, --help                    prints this help message and exit\n");
            printf("  -c, --command-line            starts with command line interface, default\n");
            printf("  -g, --graphical               after initialization starts graphical interface\n");
            printf("  -v, --validate                validate syntax of commands; use with \"< FILE\" to validate control file \n");
            return 0;
        }
    }
    
    int ret_code = 0;
    //if (UI !=3) ret_code = init();
    if ( ret_code != 0 ){ 
        FailExit(ret_code);
        return ret_code;
    }
    
    if (UI == 1) ret_code = run_cmd(1);
    if (UI == 2) ret_code = run_gui();
    if (UI == 3) ret_code = run_cmd(3);
    
    if ( ret_code != 0 ) {
        FailExit(ret_code);
        return ret_code;
    }
    
    exit();
    return 0;
}

