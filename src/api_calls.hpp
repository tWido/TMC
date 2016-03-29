#ifndef API_CALLS
#define API_CALLS

#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "api.hpp"
#include "log.hpp"

#define MAX_RESPONSE_SIZE 128

#define FT_ERROR -6
#define DEVICE_ERROR -7
#define FATAL_ERROR -8

#define MOVED_HOME_STATUS 3
#define MOVE_COMPLETED_STATUS 2
#define MOVE_STOPPED_STATUS 4

#define OTHER_MESSAGE 5
#define EMPTY 1

#define INVALID_DEST -10
#define INVALID_SOURCE -11
#define INVALID_CHANNEL -12

#define INVALID_PARAM_1 -15
#define INVALID_PARAM_2 -16
#define INVALID_PARAM_3 -17
#define INVALID_PARAM_4 -18
#define INVALID_PARAM_5 -19

#define READ_REST(x) ftStatus = FT_Read(handle, &buff[2], x, NULL); \
        if (ftStatus != FT_OK) {                                    \
        printf("FT_Error occured, error code :%d", ftStatus );      \
        return FT_ERROR;                                            \
        }  

#define EMPTY_IN_QUEUE ret = EmptyIncomingQueue(handle);    \
        if (ret != 0 ) return ret;

#define CHECK_ADDR_PARAMS(source, dest, chanID) int ret;        \
        ret = CheckParams(source,dest,chanID);                  \
         if (ret != 0) return ret;

#define GET_MESS(req_mess_class, buff_size, get_mess_code, get_mess_class  ) \
        CHECK_ADDR_PARAMS(source ,dest, channel)                \
        EMPTY_IN_QUEUE                                          \
        req_mess_class mes(dest, source, channel);              \
        SendMessage(mes, handle);                               \
        uint8_t *buff = (uint8_t *) malloc(buff_size);          \
        ret = GetResponseMess(handle, get_mess_code, buff_size, buff); \
        get_mess_class mess(buff);                              \
        *message = mess;                                        \
        free(buff);                                             \
        if ( ret != 0) return ret;                              \
        EMPTY_IN_QUEUE                                          

uint8_t DefaultDest(){
    if (opened_device.bays != -1 ) return 0x21;
    else return 0x50;
}

uint8_t DefaultSource(){
    return 0x01;
}

uint8_t DefaultChanel8(){
    return 0x01;
}

uint16_t DefaultChanel16(){
    return 0x01;
}

uint8_t DeafultRate(){
    return 1;
}

int OpenDevice(unsigned int index){
    if (index <=0 || index > devices_connected) return INVALID_PARAM_1;
    opened_device = connected_device[index-1];
    return 0;
};

int CheckParams(uint8_t source, uint8_t dest, int chanID){
    if (source != 0x01 ) return INVALID_SOURCE;
    if (chanID > opened_device.channels && chanID != -1) return INVALID_CHANNEL;
    if (dest == 0x11 || dest == 0x50) return 0;
    switch (chanID){
        case 0x21: {
            if (opened_device.bays >= 1 && opened_device.bay_used[0]) return 0;
            else return INVALID_DEST;
        }
        case 0x22:{
            if (opened_device.bays >= 2 && opened_device.bay_used[1]) return 0;
            else return INVALID_DEST;
        }
        case 0x23: {
            if (opened_device.bays == 3 && opened_device.bay_used[2]) return 0;
            else return INVALID_DEST;
        }
        default : return INVALID_DEST;
    };
    return 0;
};

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

int CheckIncomingQueue(FT_HANDLE &handle, uint16_t *ret_msgID){
    FT_STATUS ftStatus;
    unsigned int bytes;
    ftStatus = FT_GetQueueStatus(handle, &bytes);
    if (ftStatus != FT_OK ) {
        printf("FT_Error occured, error code :%d", ftStatus );
        return FT_ERROR;
    }
    if (bytes == 0 ) return EMPTY;
    uint8_t *buff = (uint8_t *) malloc(MAX_RESPONSE_SIZE);
    unsigned int red;
    ftStatus = FT_Read(handle, buff, 2, &red);          
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
            printf("Device with serial %s disconnecting\n", opened_device.SN);
            free(buff);
            return FATAL_ERROR;
        }
        case HW_RESPONSE:{
            READ_REST(4)
            HwResponse response(buff);
            printf("Device with serial %s encountered error\n", opened_device.SN);
            free(buff);
            return DEVICE_ERROR;
        }
        case RICHRESPONSE:{
            READ_REST(72)
            HwResponseInfo response(buff);      
            printf("Device with serial %s encountered error\n", opened_device.SN);
            printf("Detailed description of error \n ");
            uint16_t error_cause = response.GetMsgID();
            if (error_cause != 0) printf("\tMessage causing error: %d\n ", error_cause);
            printf("\tThorlabs error code: %d \n", response.GetCode());
            printf("\tDescription: %s\n", response.GetDescription());
            LOG("Device encountered error: ")
            LOG(response.GetDescription())
            free(buff);
            return DEVICE_ERROR;
        }
        case MOVE_HOMED:{
            READ_REST(4)
            MovedHome response(buff);
            opened_device.motor[response.GetMotorID()].homing=false;
            printf("Motor with id %d moved to home position\n", response.GetMotorID() + 1);
            free(buff);
            return MOVED_HOME_STATUS;
        }
        case MOVE_COMPLETED:{
            READ_REST(18) // 14 bytes for status updates
            MoveCompleted response(buff);
            opened_device.motor[response.GetMotorID()].homing=false;
            printf("Motor with id %d completed move\n", response.GetMotorID() + 1);
            free(buff);
            return MOVE_COMPLETED_STATUS;
        }
        case MOVE_STOPPED:{
            READ_REST(18) // 14 bytes for status updates
            MoveStopped response(buff);
           opened_device.motor[response.GetMotorID()].homing=false;
            printf("Motor with id %d stopped \n", response.GetMotorID() +1 );
            free(buff);
            return MOVE_STOPPED_STATUS;
        }
        case GET_STATUSUPDATE:{
            READ_REST(18)
            GetStatusUpdate response(buff);
            opened_device.motor[response.GetMotorID()].status_enc_count = response.GetEncCount();
            opened_device.motor[response.GetMotorID()].status_position = response.GetPosition();
            opened_device.motor[response.GetMotorID()].status_status_bits = response.GetStatusBits();   
            free(buff);
            return 0;
        }
        case GET_DCSTATUSUPDATE:{
            READ_REST(18)
            GetMotChanStatusUpdate response(buff);
            opened_device.motor[response.GetMotorID()].status_velocity = response.GetVelocity();
            opened_device.motor[response.GetMotorID()].status_position = response.GetPosition();
            opened_device.motor[response.GetMotorID()].status_status_bits = response.GetStatusBits();
            free(buff);
            return 0;
        }
        default: {
            *ret_msgID = msgID;
            free(buff);
            return OTHER_MESSAGE;
        } 
    };
}

int EmptyIncomingQueue(FT_HANDLE &handle){
    while(true){
        int ret = CheckIncomingQueue(handle, NULL);
        if (ret == EMPTY) return 0;
        if (ret == MOVED_HOME_STATUS || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
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

int GetResponseMess(FT_HANDLE &handle, uint16_t expected_msg, int size, uint8_t *mess ){
    int ret;
    uint16_t msgID;
    while(true){
        ret = CheckIncomingQueue(handle, &msgID);
        if (ret == OTHER_MESSAGE){
            if (msgID == expected_msg) {
                *((int16_t *) &mess[0]) =  htole16(msgID);
                unsigned int red;
                FT_STATUS read_status = FT_Read(handle, &mess[2], size-2, &red);
                if ( read_status != FT_OK ) {
                    printf("FT_Error occured, error code :%d\n", read_status );
                    return FT_ERROR;
                }
                return 0;
            }
            else return FATAL_ERROR;
        } 
        if (ret == MOVED_HOME_STATUS || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
        switch(ret){
            case FATAL_ERROR: return FATAL_ERROR;
            case FT_ERROR: return FT_ERROR;
            case DEVICE_ERROR: return DEVICE_ERROR;
        }
    }
    return 0;
}

namespace device_calls{
// ------------------------- Generic device calls ------------------------------

int Identify( FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource() ){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    IdentifyMs mes(dest, source);
    SendMessage(mes, handle); 
    EMPTY_IN_QUEUE
    return 0;
}

int EnableChannel(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t chanel = DefaultChanel8()){   
    CHECK_ADDR_PARAMS(source ,dest, chanel)
    EMPTY_IN_QUEUE
    SetChannelState mes(chanel, 1, dest, source);
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0;
}

int DisableChannel(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t chanel = DefaultChanel8()){  
    CHECK_ADDR_PARAMS(source ,dest, chanel)
    EMPTY_IN_QUEUE
    SetChannelState mes(chanel, 2, dest, source);
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0;
}

int ChannelState(FT_HANDLE &handle, GetChannelState *info, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t chanel = DefaultChanel8()){ 
    CHECK_ADDR_PARAMS(source ,dest, chanel)
    EMPTY_IN_QUEUE
    ReqChannelState mes(chanel, dest, source);
    SendMessage(mes, handle);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess(handle, GET_CHANENABLESTATE, HEADER_SIZE , buff);
    GetChannelState mess(buff);
    *info = mess;
    free(buff);
    if ( ret != 0) return ret;  
    EMPTY_IN_QUEUE
    return 0;
}

int DisconnectHW(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    HwDisconnect mes(dest,source);
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0;
}

int StartUpdateMess(FT_HANDLE &handle, uint8_t rate = DeafultRate(), uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    StartUpdateMessages mes(dest,source);
    if (mes.SetUpdaterate(rate) == IGNORED_PARAM) printf("This parameter is ignored in connected device. Using default.\n");
    SendMessage(mes, handle);
    opened_device.end_of_move_messages = true;
    EMPTY_IN_QUEUE
    return 0;
}

int StopUpdateMess(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    StopUpdateMessages mes(dest,source);
    SendMessage(mes, handle);
    opened_device.end_of_move_messages = false;
    EMPTY_IN_QUEUE
    return 0;
}

int GetHwInfo(FT_HANDLE &handle, HwInfo *message, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    ReqHwInfo mes(dest,source);
    SendMessage(mes, handle);
    uint8_t *buff = (uint8_t *) malloc(90);
    ret = GetResponseMess(handle, HW_GET_INFO, 90, buff);
    HwInfo info(buff);
    *message = info; 
    free(buff);
    if ( ret != 0) return ret;
    EMPTY_IN_QUEUE
    return 0;
}

int GetBayUsed(FT_HANDLE &handle, GetRackBayUsed *message, uint8_t bayID, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    ReqRackBayUsed mes(dest,source);
    mes.SetBayIdent(bayID);
    SendMessage(mes, handle);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess(handle, RACK_GET_BAYUSED, HEADER_SIZE , buff);
    GetRackBayUsed bayused(buff);
    *message = bayused;
    if ( ret != 0) return ret;
    free(buff);
    EMPTY_IN_QUEUE
    return 0;
}

int GetHubUsed(FT_HANDLE &handle, GetHubBayUsed *message, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1);
    EMPTY_IN_QUEUE
    ReqHubBayUsed mes(dest,source);
    SendMessage(mes, handle);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess(handle, HUB_GET_BAYUSED, HEADER_SIZE , buff);
    GetHubBayUsed hubused(buff);
    *message = hubused;
    if ( ret != 0) return ret;
    free(buff);
    EMPTY_IN_QUEUE
    return 0;
}

//-------------------------- Motor control calls ------------------------------

int FlashProgYes(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    YesFlashProg mes(dest,source);
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0;
};

int FlashProgNo(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    NoFlashProg mes(dest,source);
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0;
};

int SetPositionCounter(FT_HANDLE &handle, int32_t pos, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetPosCounter mes(dest, source, channel);
    if ( mes.SetPosition(pos) == INVALID_PARAM ) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return ret; //return WARNING
};

int GetPositionCounter(FT_HANDLE &handle, GetPosCounter *message, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqPosCounter,12,GET_POSCOUNTER,GetPosCounter)      
    return 0;
};

int SetEncoderCounter(FT_HANDLE &handle, int32_t count, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetEncCount mes(dest, source, channel);
    if ( mes.SetEncoderCount(count)== INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return ret; //return WARNING
};

int GetEncoderCounter(FT_HANDLE &handle, GetEncCount *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqEncCount,12,GET_ENCCOUNTER,GetEncCount)      
    return 0;
};

int SetVelocityP(FT_HANDLE &handle, int32_t acc, int32_t maxVel, 
        uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetVelocityParams mes(dest, source, channel);
    if (mes.SetAcceleration(acc) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetMaxVel(maxVel) == INVALID_PARAM) return INVALID_PARAM_2;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0;        
}

int GetVelocityP(FT_HANDLE &handle, GetVelocityParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqVelocityParams,20,GET_VELPARAMS,GetVelocityParams)       
    return 0;
};

int SetJogP(FT_HANDLE &handle, uint16_t mode, int32_t stepSize, int32_t vel, int32_t acc, uint16_t stopMode,
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetJogParams mes(dest, source, channel);
    if (mes.SetJogMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    mes.SetStepSize(stepSize);
    if (mes.SetMaxVelocity(vel) == INVALID_PARAM) return INVALID_PARAM_3;
    if (mes.SetAcceleration(acc) == INVALID_PARAM) return INVALID_PARAM_4;
    if (mes.SetStopMode(stopMode) == INVALID_PARAM) return INVALID_PARAM_5;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE
    return 0; 
};

int GetJogP(FT_HANDLE &handle, GetJogParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqJogParams,28,GET_VELPARAMS,GetJogParams)
    return 0;
};

int SetPowerUsed(FT_HANDLE &handle, uint16_t rest_power, uint16_t move_power,
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetPowerParams mes(dest, source, channel);
    if ( mes.SetRestFactor(rest_power) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetMoveFactor(move_power) == INVALID_PARAM )return INVALID_PARAM_2;        
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE        
    return 0;
};

int GetPowerUsed(FT_HANDLE &handle, GetPowerParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqPowerParams,12,GET_POWERPARAMS,GetPowerParams)
    return 0;
};

int SetBacklashDist(FT_HANDLE &handle, uint32_t dist, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetGeneralMoveParams mes(dest, source, channel);
    mes.SetBacklashDist(dist);    
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE        
    return 0;
};

int GetBacklashDist(FT_HANDLE &handle, GetGeneralMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqGeneralMoveParams,12,GET_GENMOVEPARAMS,GetGeneralMoveParams)                        
    return 0;
};

int SetRelativeMoveP(FT_HANDLE &handle, uint32_t dist, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetRelativeMoveParams mes(dest, source, channel);
    mes.SetRelativeDist(dist);    
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;        
};

int GetRelativeMoveP(FT_HANDLE &handle, GetRelativeMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqRelativeMoveParams,12,GET_MOVERELPARAMS,GetRelativeMoveParams) 
    return 0;
};

int SetAbsoluteMoveP(FT_HANDLE &handle, uint32_t pos, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetAbsoluteMoveParams mes(dest, source, channel);
    mes.SetAbsolutePos(pos);    
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetAbsoluteMoveP(FT_HANDLE &handle, GetAbsoluteMoveParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqAbsoluteMoveParams,12,GET_MOVEABSPARAMS,GetAbsoluteMoveParams) 
    return 0;
};

int SetHomingVel(FT_HANDLE &handle, uint32_t vel, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetHomeParams mes(dest, source, channel);
    if (mes.SetHomingVelocity(vel) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetHomingVel(FT_HANDLE &handle, GetHomeParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqHomeParams,20,GET_HOMEPARAMS,GetHomeParams) 
    return 0;
};

int SetLimitSwitchConfig(FT_HANDLE &handle, uint16_t CwHwLim, uint16_t CCwHwLim, uint16_t CwSwLim, uint16_t CCwSwLim, uint16_t mode, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetLimitSwitchParams mes(dest, source, channel);
    if (mes.SetClockwiseHardLimit(CwHwLim) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetCounterlockwiseHardLimit(CCwHwLim) == INVALID_PARAM) return INVALID_PARAM_2;
    if (mes.SetClockwiseSoftLimit(CwSwLim) == IGNORED_PARAM) return IGNORED_PARAM;
    if (mes.SetCounterlockwiseSoftLimit(CCwSwLim)== IGNORED_PARAM) return IGNORED_PARAM;
    ret = mes.SetLimitMode(mode);
    if (ret == INVALID_PARAM) return INVALID_PARAM_5;
    if (ret == IGNORED_PARAM) return IGNORED_PARAM;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetLimitSwitchConfig(FT_HANDLE &handle, GetLimitSwitchParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqLimitSwitchParams,22,GET_LIMSWITCHPARAMS,GetLimitSwitchParams) 
    return 0;
};

int MoveToHome(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    MoveHome mes(dest,source,channel);        
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].homing=true;  
    EMPTY_IN_QUEUE 
    return 0;
};

int StartSetRelativeMove(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    MoveRelative1 mes(dest,source,channel);        
    SendMessage(mes, handle);
     opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int StartRelativeMove(FT_HANDLE &handle, int32_t dist, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    MoveRelative2 mes(dest,source,channel);
    mes.SetRelativeDistance(dist);
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int StartSetAbsoluteMove(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    MoveAbsolute1 mes(dest,source,channel);        
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int StartAbsoluteMove(FT_HANDLE &handle, int32_t pos, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE       
    MoveAbsolute2 mes(dest,source,channel);  
    if (mes.SetAbsoluteDistance(pos) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int StartJogMove(FT_HANDLE &handle, uint8_t direction, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE       
    JogMove mes(dest,source,channel);  
    if (mes.SetDirection(direction) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int StartSetVelocityMove(FT_HANDLE &handle, uint8_t direction, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE        
    MovewVelocity mes(dest,source,channel);  
    if (mes.SetDirection(direction) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int StopMovement(FT_HANDLE &handle, uint8_t stopMode, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    StopMove mes(dest,source,channel);  
    if (mes.SetStopMode(stopMode) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    opened_device.motor[mes.GetMotorID()].stopping=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int SetAccelerationProfile(FT_HANDLE &handle, uint16_t index, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetBowIndex mes(dest, source, channel);
    if (mes.SetBowindex(index) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetAccelerationProfile(FT_HANDLE &handle, GetBowIndex *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqBowIndex,10,GET_BOWINDEX,GetBowIndex) 
    return 0;
};

int SetLedP(FT_HANDLE &handle, uint16_t mode, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetLedMode mes(dest, source, channel);
    if (mes.SetMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};


int GetLedP(FT_HANDLE &handle, GetLedMode *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqLedMode,10,GET_AVMODES,GetLedMode) 
    return 0;
};

int SetButtons(FT_HANDLE &handle, uint16_t mode, int32_t pos1, int32_t pos2, uint16_t timeout, 
        int8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint16_t channel = DefaultChanel16()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetButtonParams mes(dest, source, channel);
    if (mes.SetMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetPosition1(pos1) == INVALID_PARAM) return INVALID_PARAM_2;
    if (mes.SetPosition2(pos2) == INVALID_PARAM) return INVALID_PARAM_3;
    if (mes.SetTimeout(timeout) == IGNORED_PARAM ) return IGNORED_PARAM;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetButtonsInfo(FT_HANDLE &handle, GetButtonParams *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqButtonParams,22,GET_BUTTONPARAMS,GetButtonParams) 
    return 0;
};

int GetStatus(FT_HANDLE &handle, GetStatusUpdate *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqStatusUpdate,20,GET_STATUSUPDATE,GetStatusUpdate) 
    return 0;
};

int GetDcStatus(FT_HANDLE &handle, GetMotChanStatusUpdate *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqMotChanStatusUpdate,20,GET_DCSTATUSUPDATE,GetMotChanStatusUpdate) 
    return 0;
};

int SendServerAlive(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    ServerAlive mes(dest,source);
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetStatBits(FT_HANDLE &handle, GetStatusBits *message ,uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqStatusBits,12,GET_STATUSBITS,GetStatusBits) 
    return 0;
};

int DisableEomMessages(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    DisableEndMoveMessages mes(dest,source);
    SendMessage(mes, handle);
    opened_device.end_of_move_messages = false;
    EMPTY_IN_QUEUE 
    return 0;
};

int EnableEomMessages(FT_HANDLE &handle, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource()){
    CHECK_ADDR_PARAMS(source ,dest, -1)
    EMPTY_IN_QUEUE
    EnableEndMoveMessages mes(dest,source);
    SendMessage(mes, handle);
    opened_device.end_of_move_messages = false;
    EMPTY_IN_QUEUE 
    return 0;
};

int CreateTrigger(FT_HANDLE &handle, uint8_t mode, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    CHECK_ADDR_PARAMS(source ,dest, channel)
    EMPTY_IN_QUEUE
    SetTrigger mes(dest,source, channel);
    if (mes.SetMode(mode) == IGNORED_PARAM) return IGNORED_PARAM;
    SendMessage(mes, handle);
    EMPTY_IN_QUEUE 
    return 0;
};

int GetMotorTrigger(FT_HANDLE &handle, GetTrigger *message, uint8_t dest = DefaultDest(), uint8_t source = DefaultSource(), uint8_t channel = DefaultChanel8()){
    GET_MESS(ReqTrigger,HEADER_SIZE,GET_TRIGGER,GetTrigger) 
    return 0;
};


} // namespace device_calls

#endif 