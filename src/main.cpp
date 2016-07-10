#include <cstdlib>
#include <iostream>
#include "init.cpp"
#include "cmd.cpp"
#ifdef _GUI_            
#include "gui.hpp"
#endif

using namespace std;

int UI = 1; // 1 -> cmd, 2-> graphical, 3 - > validate file
        
void FailExit(int ret_code){

switch (ret_code){
        case STOP : {
            freeResources();
            break;
        }
        case SYSTEM_ERROR : {
            printf("Encountered system error.\n");
            freeResources();
            break;
        }
        case FT_ERROR : {
            printf("Encountered error with FTDI library.\n");
            freeResources();
            break;
        }
        case DEVICE_ERROR : {
            printf("Motor device encountered error.\n");
            freeResources();
            break;
        }
        case FATAL_ERROR :{
            printf("Fatal error.\n");
            freeResources();
            break;
        }
    }

}

void exit_signal(int signum){
    if (signum == SIGINT){
        freeResources();
        exit(-1);
    }
} 

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0) {
            printf("Options\n");
            printf("  -h, --help                    prints this help message and exit\n");
            printf("  -c, --command-line            starts with command line interface, default\n");
            #ifdef _GUI_
            printf("  -g, --graphical               after initialization starts graphical interface\n");
            #endif
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
            #ifdef _GUI_
            printf("  -g, --graphical               after initialization starts graphical interface\n");
            #endif
            printf("  -v, --validate                validate syntax of commands; use with \"< FILE\" to validate control file \n");
            return 0;
        }
    }
    
    int ret_code = 0;
    if (UI !=3) ret_code = init();
    if ( ret_code != 0 ){ 
        FailExit(ret_code);
        return ret_code;
    }
    signal(SIGINT, exit_signal);
    if (UI == 1) ret_code = run_cmd(1);
    #ifdef _GUI_
    if (UI == 2) run_gui();
    #endif
    if (UI == 3){
        connected_device = new controller_device[1];
        ret_code = run_cmd(3);
    }
    
    if ( ret_code != 0 ) {
        FailExit(ret_code);
        return ret_code;
    }

    freeResources();    
    return 0;
}

