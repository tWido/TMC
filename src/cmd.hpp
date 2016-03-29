#ifndef CMD_HPP
#define	CMD_HPP
#include "api_calls.hpp"
#include "device.hpp"


int run_cmd(){
    device_calls::Identify(opened_device.handle);
    return 0;
}

#endif	/* CMD_HPP */

