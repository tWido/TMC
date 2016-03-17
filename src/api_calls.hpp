#ifndef API_CALLS
#define API_CALLS

#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "api.hpp"

#define MAX_RESPONSE_SIZE 128
#define FT_ERROR -4

uint8_t DefaultDest(){
    return 0x50;
}

uint8_t DefaultSource(){
    return 0x01;
}

uint8_t DefaultChanel(){
    return 0x01;
}

uint8_t DeafultRate(){
    return 1;
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

int CheckIncomingQueue(FT_HANDLE &handle){
    FT_STATUS ftStatus;
    unsigned int bytes;
    ftStatus = FT_GetQueueStatus(handle, &bytes);
    if (ftStatus != FT_OK ) {
        printf("FT_Error occured, error code :%d", ftStatus );
        return FT_ERROR;
    }
    if (bytes == 0 ) return 0;
    uint8_t *buff = (uint8_t *) malloc(MAX_RESPONSE_SIZE);
    ftStatus = FT_Read(handle, buff, 2, NULL);          
    if (ftStatus != FT_OK) {
        printf("FT_Error occured, error code :%d", ftStatus );
        return FT_ERROR;
    }
    uint16_t msgID = le16toh(*((uint16_t*) &buff[0])); 
    switch ( msgID ){
        case HW_DISCONNECT: 
            //device wants disconnect
            break;
        case HW_RESPONSE:
            //error occured
            break;
        case RICHRESPONSE:
            //error occured, got response message
            break;
        case MOVE_HOMED:
            //moved to home position
            break;
        case MOVE_COMPLETED:
            //move completed
            break;
        case MOVE_STOPPED:
            //stopped
            break;
        case GET_STATUSUPDATE:
            //status update
            break;
        case GET_DCSTATUSUPDATE:
            //status update
            break;
        default: 
            printf("Unexpected response");
            return -1;
    };
    
    return -1;
}

int GetResponseMess(uint16_t last_msg_sent, uint16_t expected_msg, uint8_t *mess ){
    //not implemented
    //check incoming queue for error messages, return fail on error, 
    // other messages handled
    // expected message loaded into given pointer
    return 0;
}

int Identify( FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource() ){
    CheckIncomingQueue(handle);
    IdentifyMs mes(dest, source);
    SendMessage(mes, handle); 
    CheckIncomingQueue(handle);
    return 0;
}

int EnableChannel(FT_HANDLE &handle, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){   
    CheckIncomingQueue(handle);
    SetChannelState mes(chanel, 1, dest, source);
    SendMessage(mes, handle);
    CheckIncomingQueue(handle);
    return 0;
}

int DisableChannel(FT_HANDLE &handle,uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){  
    CheckIncomingQueue(handle);
    SetChannelState mes(chanel, 2, dest, source);
    SendMessage(mes, handle);
    CheckIncomingQueue(handle);
    return 0;
}

int ChannelState(FT_HANDLE &handle, GetChannelState *info, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){ 
    CheckIncomingQueue(handle);
    ReqChannelState mes(chanel, dest, source);
    SendMessage(mes, handle);
    uint8_t *ret = NULL;
    if (GetResponseMess(REQ_CHANENABLESTATE, GET_CHANENABLESTATE, ret) != 0) return -1;
    info = new GetChannelState(ret);
    return 0;
}

int DisconnectHW(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CheckIncomingQueue(handle);
    HwDisconnect mes(dest,source);
    SendMessage(mes, handle);
    CheckIncomingQueue(handle);
    return 0;
}

int StartUpdateMess(FT_HANDLE &handle, uint8_t rate = DeafultRate(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CheckIncomingQueue(handle);
    StartUpdateMessages mes(dest,source);
    if (mes.SetUpdaterate(rate) == IGNORED_PARAM) printf("This parameter is ignored in connected device. Using default.\n");
    SendMessage(mes, handle);
    CheckIncomingQueue(handle);
    return 0;
}

int StopUpdateMess(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CheckIncomingQueue(handle);
    StopUpdateMessages mes(dest,source);
    SendMessage(mes, handle);
    CheckIncomingQueue(handle);
    return 0;
}

#endif 