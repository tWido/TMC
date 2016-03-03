#include <cstdlib>
#include <iostream>
#include "api_calls.hpp"
#include "../ftdi_lib/ftd2xx.h"
#include "../ftdi_lib/WinTypes.h"


using namespace std;

int OpenDevice(){
    //not implemented, FT_OpenEx, set communication
    return -1;
}


int main(int argc, char** argv) {
    init();
    
    free(connected_device);
    return 0;
}

