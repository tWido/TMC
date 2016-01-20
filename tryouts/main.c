/*
	To build use the following gcc statement 
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o read main.c -L. -lftd2xx -Wl,-rpath,/usr/local/lib
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ftd2xx.h"
#include <unistd.h>
#include <stdint.h>
#include <string.h>


int main(int argc, char *argv[])
{
    FT_STATUS	ftStatus;
    FT_HANDLE	ftHandle0;
    int iport;
    int retCode = 0;

    iport = 0;
 
    DWORD Vid  = 0x0403;
    DWORD Pid  = 0xFAF0;
    FT_SetVIDPID(Vid, Pid);
    
    unsigned int numdevs = 0;
    ftStatus =  FT_CreateDeviceInfoList(&numdevs);
    if (ftStatus != FT_OK) {
        printf("creating device list failed, err: %d ", ftStatus );
        return -1;
    }
    printf("device number: %d", numdevs );
    
    FT_DEVICE_LIST_INFO_NODE *list = (FT_DEVICE_LIST_INFO_NODE*) malloc(numdevs*sizeof(FT_DEVICE_LIST_INFO_NODE));
    
    unsigned int i = 0;
    for (  ; i< numdevs; i++){
        printf(" description: %s", list[i].Description );
        printf("flags:  %d", list[i].Flags );
        printf("id: %d", list[i].ID );
        printf("locID(shouldn't work): %d", list[i].LocId );
        printf("SN: %s", list[i].SerialNumber );
        printf("type: %d", list[i].Type );
    }
    
    printf("Opening port %d\n", iport);
    ftStatus = FT_Open(iport, &ftHandle0);
    if(ftStatus != FT_OK) {
            /* 
                    This can fail if the ftdi_sio driver is loaded
                    use lsmod to check this and rmmod ftdi_sio to remove
                    also rmmod usbserial
             */
            printf("FT_Open(%d) failed\n", iport);
            return 1;
    }

    printf("FT_Open succeeded.  Handle is %p\n", ftHandle0);
    
    ftStatus = FT_SetBaudRate(ftHandle0, 115200);
    if (ftStatus != FT_OK) {
        printf("setting baud rate failed");
        return -1;
    }
    printf("baud rate set\n");
    
    ftStatus = FT_SetDataCharacteristics(ftHandle0, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    if (ftStatus != FT_OK) {
        printf("setting data characteristics failed");
        return -1;
    }
    printf("data characteristics set\n");
    usleep(50);
    
    ftStatus = FT_Purge(ftHandle0, FT_PURGE_RX | FT_PURGE_TX);
    if (ftStatus != FT_OK) {
        printf("purging device failed");
        return -1;
    }
    printf("device purged\n");
    usleep(50);
    ftStatus = FT_ResetDevice(ftHandle0);
    if (ftStatus != FT_OK) {
        printf("reseting device failed");
        return -1;
    }
    printf("device reset\n");
    
    ftStatus = FT_SetFlowControl(ftHandle0, FT_FLOW_RTS_CTS, 0, 0);
    if (ftStatus != FT_OK) {
        printf("seting flow control failed");
        return -1;
    }
    printf("flow control set\n");
    
    ftStatus = FT_SetRts(ftHandle0);
    if (ftStatus != FT_OK) {
        printf("seting rts failed");
        return -1;
    }
    printf("rts set\n");
    
    
    
/*
    //flash led
    uint8_t message[6];
    message[4] = 0x50; //dest
    message[5] = 0x01; //source
    message[0] = 0x23; // id
    message[1] = 0x02; //id
    message[2] = 0; //arg1
    message[3] = 0; //arg2
*/

    // request info
    uint8_t message[6];
    message[4] = 0x50; //dest
    message[5] = 0x01; //source
    message[0] = 0x05; // id
    message[1] = 0; //id
    message[2] = 0; //arg1
    message[3] = 0; //arg2

    FT_SetTimeouts(ftHandle0, 1000,1000);
    unsigned int wrote;
    printf("something\n");
    ftStatus  = FT_Write(ftHandle0, message, 6, &wrote);
    if (ftStatus != FT_OK) {
        printf("FAAAAIIIIILL");
    }
    else {
        printf("OK\n");
    }
    printf("wrote %d bytes \n", wrote);
    

    //get info
    uint8_t buff[90];
    unsigned int red;
    ftStatus = FT_Read(ftHandle0, buff, 89, &red );
    if (ftStatus != FT_OK) {
        printf("FAAAAIIIIILL");
    }
    else {
        printf("OK\n");
    }
    printf("red %d bytes\n", red);
    
    //int i = 0;
    for (i = 0; i< red; i++){
        printf(" %d", buff[i]);
    }
    char sn[5];
    memcpy( (void*)sn, (void*)&buff[6], 4);
    sn[5] = '\0';
    printf("sn : %s \n", sn);
    
    FT_Close(ftHandle0);
    printf("Returning %d\n", retCode);
    return retCode;
}
