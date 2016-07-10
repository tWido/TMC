#ifndef INIT
#define INIT
/*
 * Startup functions. Finding devices, initializing
 */
#include "api.hpp"
#include "../ftdi_lib/ftd2xx.h"

#define STOP 1
#define SYSTEM_ERROR -3
#define FIND_DEV(code) if (strncmp(name.c_str(), #code, 6) == 0){ printf("Found controller device, type: %s\n", name.c_str()) ;  return code;}

#define USB_PATH path = dev_path; path.append(loc);

using namespace std;

extern int addVidPid();

extern int RemoveModules(std::string module_name);

extern void LoadNone(controller_device &device, int mot_id);

extern int LoadRestrictions(controller_device &device, std::string device_name);

extern int ToDevType(std::string name);

extern int Bays(int type);

extern int Channels(int type);

extern functions_set functionsSet(int devtype);

extern int LoadDeviceInfo( controller_device &device);

extern int init();

extern void freeResources();

#endif