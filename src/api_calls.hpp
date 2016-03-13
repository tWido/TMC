#ifndef API_CALLS
#define API_CALLS

#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "api.hpp"

uint8_t DefaultDest(){
    return 0x50;
}

uint8_t DefaultSource(){
    return 0x01;
}

uint8_t DefaultChanel(){
    return 0x01;
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

int CheckIcomingQueue(){
    //not implemented
    //check if queue is empty, process possible messages
    return -1;
}

int GetResponseMess(uint16_t last_msg, uint16_t expected_msg, uint8_t *mess ){
    //not implemented
    //check incoming queue for error messages, return fail on error, 
    // other messages handled
    // expected message loaded into given pointer
    return 0;
}

int Identify( FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource() ){
    CheckIcomingQueue();
    IdentifyMs mes(dest, source);
    SendMessage(mes, handle); 
    CheckIcomingQueue();
    return 0;
}

int EnableChannel(FT_HANDLE &handle, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){   
    CheckIcomingQueue();
    SetChannelState mes(chanel, 1, dest, source);
    SendMessage(mes, handle);
    CheckIcomingQueue();
    return 0;
}

int DisableChannel(FT_HANDLE &handle,uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){  
    CheckIcomingQueue();
    SetChannelState mes(chanel, 2, dest, source);
    SendMessage(mes, handle);
    CheckIcomingQueue();
    return 0;
}

int ChannelState(FT_HANDLE &handle, GetChannelState *info, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){ 
    CheckIcomingQueue();
    ReqChannelState mes(chanel, dest, source);
    SendMessage(mes, handle);
    uint8_t *ret;
    if (GetResponseMess(REQ_CHANENABLESTATE, GET_CHANENABLESTATE, ret) != 0) return -1;
    info = new GetChannelState(ret);
    return 0;
}

#endif 