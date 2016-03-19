#ifndef API_CALLS
#define API_CALLS

#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "api.hpp"

#define MAX_RESPONSE_SIZE 128
#define FT_ERROR -4
#define DEVICE_ERROR -5
#define FATAL_ERROR -6
#define MOVED_HOME 3
#define MOVE_COMPLETED_STATUS 2
#define MOVE_STOPPED_STATUS 4
#define OTHER_MESSAGE 5
#define EMPTY 1

#define READ_REST(x) ftStatus = FT_Read(handle, &buff[2], x, NULL);   if (ftStatus != FT_OK) {printf("FT_Error occured, error code :%d", ftStatus );   return FT_ERROR; }  

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
    return FT_ERROR;
}

int CheckIncomingQueue(FT_HANDLE &handle, controller_device &device, uint16_t *ret_msgID){
    FT_STATUS ftStatus;
    unsigned int bytes;
    ftStatus = FT_GetQueueStatus(handle, &bytes);
    if (ftStatus != FT_OK ) {
        printf("FT_Error occured, error code :%d", ftStatus );
        return FT_ERROR;
    }
    if (bytes == 0 ) return EMPTY;
    uint8_t *buff = (uint8_t *) malloc(MAX_RESPONSE_SIZE);
    ftStatus = FT_Read(handle, buff, 2, NULL);          
    if (ftStatus != FT_OK) {
        printf("FT_Error occured, error code :%d", ftStatus );
        free(buff);
        return FT_ERROR;
    }
    uint16_t msgID = le16toh(*((uint16_t*) &buff[0])); 
    switch ( msgID ){
        case HW_DISCONNECT: {
            READ_REST(4)
            HwDisconnect response(buff);
            printf("Device with serial %s disconnecting\n", device.SN);
            free(buff);
            return FATAL_ERROR;
        }
        case HW_RESPONSE:{
            READ_REST(4)
            HwResponse response(buff);
            printf("Device with serial %s encountered error\n", device.SN);
            free(buff);
            return DEVICE_ERROR;
        }
        case RICHRESPONSE:{
            READ_REST(72)
            HwResponseInfo response(buff);      
            printf("Device with serial %s encountered error\n", device.SN);
            printf("Detailed description of error \n ");
            uint16_t error_cause = response.GetMsgID();
            if (error_cause != 0) printf("\tMessage causing error: %d\n ", error_cause);
            printf("\tThorlabs error code: %d \n", response.GetCode());
            printf("\tDescription: %s\n", response.GetDescription());
            free(buff);
            return DEVICE_ERROR;
        }
        case MOVE_HOMED:{
            READ_REST(4)
            MovedHome response(buff);
            uint8_t motor_channel = response.GetSource();
            if (motor_channel == 0x50 ) printf("Moved to home position\n");
            else printf("Motor in bay %d moved to home position\n",  (motor_channel | 0x0F)  );
            free(buff);
            return MOVED_HOME;
        }
        case MOVE_COMPLETED:{
            READ_REST(4)
            MoveCompleted response(buff);
            uint8_t motor_channel = response.GetSource();
            if (motor_channel == 0x50 ) printf("Move completed\n");
            else printf("Motor in bay %d completed move\n",  (motor_channel | 0x0F)  );
            free(buff);
            return MOVE_COMPLETED_STATUS;
        }
        case MOVE_STOPPED:{
            READ_REST(4)
            MoveStopped response(buff);
            uint8_t motor_channel = response.GetSource();
            if (motor_channel == 0x50 ) printf("Move stopped\n");
            else printf("Motor in bay %d stopped moving\n",  (motor_channel | 0x0F)  );
            free(buff);
            return MOVE_STOPPED_STATUS;
        }
        case GET_STATUSUPDATE:{
            READ_REST(18)
            GetStatusUpdate response(buff);
            uint8_t motor_channel = response.GetSource();
            for (int i = 0; i< device.channels; i++){
                if (motor_channel == device.motor[i].dest ) {
                    device.motor[i].status_enc_count = response.GetEncCount();
                    device.motor[i].status_position = response.GetPosition();
                    device.motor[i].status_status_bits = response.GetStatusBits();
                    break;
                }
            }
            free(buff);
            return 0;
        }
        case GET_DCSTATUSUPDATE:{
            READ_REST(18)
            GetMotChanStatusUpdate response(buff);
            uint8_t motor_channel = response.GetSource();
            for (int i = 0; i< device.channels; i++){
                if (motor_channel == device.motor[i].dest ) {
                    device.motor[i].status_velocity = response.GetVelocity();
                    device.motor[i].status_position = response.GetPosition();
                    device.motor[i].status_status_bits = response.GetStatusBits();
                    break;
                }
            }
            free(buff);
            return 0;
        }
        default: {
            ret_msgID = &msgID;
            free(buff);
            return OTHER_MESSAGE;
        } 
    };
}

int EmptyIncomingQueue(FT_HANDLE &handle, controller_device &device){
    while(true){
        int ret = CheckIncomingQueue(handle, device, NULL);
        if (ret == EMPTY) return 0;
        if (ret == MOVED_HOME || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
        switch(ret){
            case FATAL_ERROR: return FATAL_ERROR;
            case FT_ERROR: return FT_ERROR;
            case DEVICE_ERROR: return DEVICE_ERROR;
            case OTHER_MESSAGE: {
                printf("Unknown message received, protocol violated\n");
                return FATAL_ERROR;
            }
        }
    }
}

int GetResponseMess(FT_HANDLE &handle, controller_device &device, uint16_t expected_msg, int size, uint8_t *mess ){
    int ret;
    uint16_t msgID;
    while(true){
        ret = CheckIncomingQueue(handle, device, &msgID);
        if (ret == OTHER_MESSAGE){
            if (msgID == expected_msg) {
                *((int16_t *) &mess[0]) =  htole16(msgID);
                FT_STATUS read_status = FT_Read(handle, &mess[2], size-2, NULL);
                if ( read_status != FT_OK ) {
                    printf("FT_Error occured, error code :%d", read_status );
                    return FT_ERROR;
                }
            }
            else return FATAL_ERROR;
        } 
        if (ret == MOVED_HOME || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
        switch(ret){
            case FATAL_ERROR: return FATAL_ERROR;
            case FT_ERROR: return FT_ERROR;
            case DEVICE_ERROR: return DEVICE_ERROR;
        }
    }
    return 0;
}

int Identify( FT_HANDLE &handle, controller_device &device, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource() ){
    EmptyIncomingQueue(handle, device);
    IdentifyMs mes(dest, source);
    SendMessage(mes, handle); 
    EmptyIncomingQueue(handle, device);
    return 0;
}

int EnableChannel(FT_HANDLE &handle, controller_device &device, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){   
    EmptyIncomingQueue(handle, device);
    SetChannelState mes(chanel, 1, dest, source);
    SendMessage(mes, handle);
    EmptyIncomingQueue(handle, device);
    return 0;
}

int DisableChannel(FT_HANDLE &handle, controller_device &device,uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){  
    EmptyIncomingQueue(handle, device);
    SetChannelState mes(chanel, 2, dest, source);
    SendMessage(mes, handle);
    EmptyIncomingQueue(handle, device);
    return 0;
}

int ChannelState(FT_HANDLE &handle, controller_device &device, GetChannelState *info, uint8_t chanel = DefaultChanel(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){ 
    EmptyIncomingQueue(handle, device);
    ReqChannelState mes(chanel, dest, source);
    SendMessage(mes, handle);
    uint8_t *ret = (uint8_t *) malloc(HEADER_SIZE);
    if (GetResponseMess(handle, device, GET_CHANENABLESTATE, HEADER_SIZE ,ret) != 0) return -1;
    info = new GetChannelState(ret);
    return 0;
}

int DisconnectHW(FT_HANDLE &handle, controller_device &device, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    EmptyIncomingQueue(handle, device);
    HwDisconnect mes(dest,source);
    SendMessage(mes, handle);
    EmptyIncomingQueue(handle, device);
    return 0;
}

int StartUpdateMess(FT_HANDLE &handle, controller_device &device, uint8_t rate = DeafultRate(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    EmptyIncomingQueue(handle, device);
    StartUpdateMessages mes(dest,source);
    if (mes.SetUpdaterate(rate) == IGNORED_PARAM) printf("This parameter is ignored in connected device. Using default.\n");
    SendMessage(mes, handle);
    EmptyIncomingQueue(handle, device);
    return 0;
}

int StopUpdateMess(FT_HANDLE &handle, controller_device &device, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    EmptyIncomingQueue(handle, device);
    StopUpdateMessages mes(dest,source);
    SendMessage(mes, handle);
    EmptyIncomingQueue(handle, device);
    return 0;
}

#endif 