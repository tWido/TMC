#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "api.hpp"

#define ERROR_FOUND -2
#define BAD_PARAMS -4
#define ERROR_RETURNED -3


defaults gdefaults;

uint8_t DefaultDest(){
    return gdefaults.dest;
}

uint8_t DefaultSource(){
    return gdefaults.source;
}

uint8_t DefaultChanel(){
    return gdefaults.chanID;
}

int CheckDeviceAbility(){
    //not implementeed
    return-1;}

int SendMessage(Message &message, FT_HANDLE &handle){
    FT_STATUS wrStatus;
    unsigned int wrote;
    wrStatus = FT_Write(handle, message.data(), message.msize(), &wrote );
    if (wrStatus == FT_OK && wrote == message.msize()){
        return 0;
    }
    else {
        printf("Sending message failed, error code : %d \n", wrStatus );
        printf("wrote : %d should write: %d \n", wrote, message.msize());
    }
    return -1;
}

int CheckMessages(uint16_t last_msg = 0, uint16_t expected_msg = 0, uint8_t *mess = NULL){
    //not implemented
    //check incoming queue for error messages, return fail on error, 
    // other messages handled
    // expected message loaded into given pointer
    return 0;
}

int Identify( FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource() ){
    if (CheckMessages(0) != 0) return ERROR_FOUND;
    IdentifyMs mes(dest, source);
    SendMessage(mes, handle); 
    if (CheckMessages(IDENTIFY) != 0) return ERROR_RETURNED;
    return 0;
}

int EnableChannel(FT_HANDLE &handle, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){   
    if (CheckMessages() != 0) return ERROR_FOUND;
    SetChannelState mes(chanel, 1, dest, source);
    SendMessage(mes, handle);
    if (CheckMessages(SET_CHANENABLESTATE) != 0) return ERROR_RETURNED;
    return 0;
}

int DisableChannel(FT_HANDLE &handle,uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){  
    if (CheckMessages(0) != 0) return ERROR_FOUND;
    SetChannelState mes(chanel, 2, dest, source);
    SendMessage(mes, handle);
    if (CheckMessages(SET_CHANENABLESTATE) != 0) return ERROR_RETURNED;
    return 0;
}

int ChannelState(FT_HANDLE &handle, GetChannelState *info, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){ 
    if (CheckMessages(0) != 0) return ERROR_FOUND;
    ReqChannelState mes(chanel, dest, source);
    SendMessage(mes, handle);
    uint8_t *ret;
    if (CheckMessages(REQ_CHANENABLESTATE, GET_CHANENABLESTATE, ret) != 0) return ERROR_RETURNED;
    info = new GetChannelState(ret);
    return 0;
}