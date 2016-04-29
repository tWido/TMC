#include "api.hpp"
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

//device globals definitions
int devices_connected = 0;
controller_device *connected_device = NULL;
controller_device opened_device;
int opened_device_index  = -1;

#define READ_REST(x)  unsigned int bytes_red; ftStatus = FT_Read(opened_device.handle, &buff[2], x, &bytes_red); \
        if (ftStatus != FT_OK) {                                    \
        fprintf(stderr,"FT_Error occured, error code :%d\n", ftStatus );    \
        return FT_ERROR;                                            \
        }  

#define EMPTY_IN_QUEUE ret = EmptyIncomingQueue();    \
        if (ret != 0 ) return ret;

#define CHECK_ADDR_PARAMS(dest, chanID) int ret;        \
        ret = CheckParams(dest,chanID);                  \
         if (ret != 0) return ret;

#define GET_MESS(req_mess_class, buff_size, get_mess_code, get_mess_class  ) \
        CHECK_ADDR_PARAMS(dest, channel)                        \
        EMPTY_IN_QUEUE                                          \
        req_mess_class mes(dest, SOURCE, channel);              \
        SendMessage(mes);                                       \
        uint8_t *buff = (uint8_t *) malloc(buff_size);          \
        ret = GetResponseMess(get_mess_code, buff_size, buff);  \
        get_mess_class mess(buff);                              \
        *message = mess;                                        \
        free(buff);                                             \
        if ( ret != 0) return ret;                              \
        EMPTY_IN_QUEUE  


int SendMessage(Message &message){
    FT_STATUS wrStatus;
    unsigned int wrote;
    wrStatus = FT_Write(opened_device.handle, message.data(), message.msize(), &wrote );
    if (wrStatus == FT_OK && wrote == message.msize()){
        return 0;
    }
    else {
        fprintf(stderr,"Sending message failed, error code : %d \n", wrStatus );
        fprintf(stderr,"wrote : %d should write: %d \n", wrote, message.msize());
    }
    return FT_ERROR;
}

int CheckParams( uint8_t dest, int chanID){
    if (chanID > opened_device.channels && chanID != -1) return INVALID_CHANNEL;
    if (dest == 0x11 || dest == 0x50) return 0;
    switch (dest){
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

int CheckIncomingQueue(uint16_t *ret_msgID){
    FT_STATUS ftStatus;
    unsigned int bytes;
    ftStatus = FT_GetQueueStatus(opened_device.handle, &bytes);
    if (ftStatus != FT_OK ) {
        fprintf(stderr,"FT_Error occured, error code :%d\n", ftStatus );
        return FT_ERROR;
    }
    if (bytes == 0 ) return EMPTY;
    uint8_t *buff = (uint8_t *) malloc(MAX_RESPONSE_SIZE);
    unsigned int red;
    ftStatus = FT_Read(opened_device.handle, buff, 2, &red);          
    if (ftStatus != FT_OK) {
        fprintf(stderr,"FT_Error occured, error code :%d\n", ftStatus );
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
            fprintf(stderr,"Device with serial %s encountered error\n", opened_device.SN);
            free(buff);
            return DEVICE_ERROR;
        }
        case RICHRESPONSE:{
            READ_REST(72)
            HwResponseInfo response(buff);      
            fprintf(stderr, "Device with serial %s encountered error\n", opened_device.SN);
            fprintf(stderr, "Detailed description of error \n ");
            uint16_t error_cause = response.GetMsgID();
            if (error_cause != 0) printf("\tMessage causing error: %d\n ", error_cause);
            fprintf(stderr, "\tThorlabs error code: %d \n", response.GetCode());
            fprintf(stderr, "\tDescription: %s\n", response.GetDescription());
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

int EmptyIncomingQueue(){
    while(true){
        int ret = CheckIncomingQueue(NULL);
        if (ret == EMPTY) return 0;
        if (ret == MOVED_HOME_STATUS || ret == MOVE_COMPLETED_STATUS || ret == MOVE_STOPPED_STATUS || ret == 0) continue; 
        switch(ret){
            case FATAL_ERROR: return FATAL_ERROR;
            case FT_ERROR: return FT_ERROR;
            case DEVICE_ERROR: return DEVICE_ERROR;
            case OTHER_MESSAGE: {
                fprintf(stderr, "Unknown message received, protocol violated\n");
                return FATAL_ERROR;
            }
        }
    }
}

int GetResponseMess(uint16_t expected_msg, int size, uint8_t *mess ){
    int ret;
    uint16_t msgID;
    while(true){
        ret = CheckIncomingQueue(&msgID);
        if (ret == OTHER_MESSAGE){
            if (msgID == expected_msg) {
                *((int16_t *) &mess[0]) =  htole16(msgID);
                unsigned int red;
                FT_STATUS read_status = FT_Read(opened_device.handle, &mess[2], size-2, &red);
                if ( read_status != FT_OK ) {
                    fprintf(stderr, "FT_Error occured, error code :%d\n", read_status );
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


int device_calls::Identify(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    IdentifyMs mes(dest, 0x01);
    SendMessage(mes); 
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::EnableChannel(uint8_t dest, uint8_t chanel){   
    CHECK_ADDR_PARAMS(dest, chanel)
    EMPTY_IN_QUEUE
    SetChannelState mes(chanel, 1, dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::DisableChannel(uint8_t dest, uint8_t chanel){  
    CHECK_ADDR_PARAMS(dest, chanel)
    EMPTY_IN_QUEUE
    SetChannelState mes(chanel, 2, dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::ChannelState(GetChannelState *info, uint8_t dest, uint8_t chanel){ 
    CHECK_ADDR_PARAMS(dest, chanel)
    EMPTY_IN_QUEUE
    ReqChannelState mes(chanel, dest, SOURCE);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess( GET_CHANENABLESTATE, HEADER_SIZE , buff);
    GetChannelState mess(buff);
    *info = mess;
    free(buff);
    if ( ret != 0) return ret;  
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::DisconnectHW(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    HwDisconnect mes(dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::StartUpdateMess(uint8_t rate, uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    StartUpdateMessages mes(dest, SOURCE);
    if (mes.SetUpdaterate(rate) == IGNORED_PARAM) printf("This parameter is ignored in connected device. Using default.\n");
    SendMessage(mes);
    opened_device.status_updates = true;
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::StopUpdateMess(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    StopUpdateMessages mes(dest, SOURCE);
    SendMessage(mes);
    opened_device.status_updates = false;
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::GetHwInfo(HwInfo *message, uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    ReqHwInfo mes(dest, SOURCE);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(90);
    ret = GetResponseMess( HW_GET_INFO, 90, buff);
    HwInfo info(buff);
    *message = info; 
    free(buff);
    if ( ret != 0) return ret;
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::GetBayUsed(GetRackBayUsed *message, uint8_t bayID, uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    ReqRackBayUsed mes(dest, SOURCE);
    mes.SetBayIdent(bayID);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess( RACK_GET_BAYUSED, HEADER_SIZE , buff);
    GetRackBayUsed bayused(buff);
    *message = bayused;
    if ( ret != 0) return ret;
    free(buff);
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::GetHubUsed(GetHubBayUsed *message, uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1);
    EMPTY_IN_QUEUE
    ReqHubBayUsed mes(dest, SOURCE);
    SendMessage(mes);
    uint8_t *buff = (uint8_t *) malloc(HEADER_SIZE);
    ret = GetResponseMess( HUB_GET_BAYUSED, HEADER_SIZE , buff);
    GetHubBayUsed hubused(buff);
    *message = hubused;
    if ( ret != 0) return ret;
    free(buff);
    EMPTY_IN_QUEUE
    return 0;
}

int device_calls::FlashProgYes(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    YesFlashProg mes(dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
};

int device_calls::FlashProgNo(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    NoFlashProg mes(dest, SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;
};

int device_calls::SetPositionCounter(int32_t pos, uint8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetPosCounter mes(dest, SOURCE, channel);
    mes.SetPosition(pos);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return ret; //return WARNING
};

int device_calls::GetPositionCounter(GetPosCounter *message, uint8_t dest, uint8_t channel){
    GET_MESS(ReqPosCounter,12,GET_POSCOUNTER,GetPosCounter)      
    return 0;
};

int device_calls::SetEncoderCounter(int32_t count, uint8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetEncCount mes(dest, SOURCE, channel);
    mes.SetEncoderCount(count);
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return ret; //return WARNING
};

int device_calls::GetEncoderCounter(GetEncCount *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqEncCount,12,GET_ENCCOUNTER,GetEncCount)      
    return 0;
};

int device_calls::SetVelocityP(int32_t acc, int32_t maxVel, uint8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetVelocityParams mes(dest, SOURCE, channel);
    if (mes.SetAcceleration(acc) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetMaxVel(maxVel) == INVALID_PARAM) return INVALID_PARAM_2;
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0;        
};

int device_calls::GetVelocityP(GetVelocityParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqVelocityParams,20,GET_VELPARAMS,GetVelocityParams)       
    return 0;
};

int device_calls::SetJogP(uint16_t mode, int32_t stepSize, int32_t vel, int32_t acc, uint16_t stopMode, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetJogParams mes(dest, SOURCE, channel);
    if (mes.SetJogMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    mes.SetStepSize(stepSize);
    if (mes.SetMaxVelocity(vel) == INVALID_PARAM) return INVALID_PARAM_3;
    if (mes.SetAcceleration(acc) == INVALID_PARAM) return INVALID_PARAM_4;
    if (mes.SetStopMode(stopMode) == INVALID_PARAM) return INVALID_PARAM_5;
    SendMessage(mes);
    EMPTY_IN_QUEUE
    return 0; 
};

int device_calls::GetJogP(GetJogParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqJogParams,28,GET_JOGPARAMS,GetJogParams)
    return 0;
};

int device_calls::SetPowerUsed(uint16_t rest_power, uint16_t move_power, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetPowerParams mes(dest, SOURCE, channel);
    if ( mes.SetRestFactor(rest_power) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetMoveFactor(move_power) == INVALID_PARAM )return INVALID_PARAM_2;        
    SendMessage(mes);
    EMPTY_IN_QUEUE        
    return 0;
};

int device_calls::GetPowerUsed(GetPowerParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqPowerParams,12,GET_POWERPARAMS,GetPowerParams)
    return 0;
};

int device_calls::SetBacklashDist(uint32_t dist, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetGeneralMoveParams mes(dest, SOURCE, channel);
    mes.SetBacklashDist(dist);    
    SendMessage(mes);
    EMPTY_IN_QUEUE        
    return 0;
};

int device_calls::GetBacklashDist(GetGeneralMoveParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqGeneralMoveParams,12,GET_GENMOVEPARAMS,GetGeneralMoveParams)                        
    return 0;
};

int device_calls::SetRelativeMoveP(uint32_t dist, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetRelativeMoveParams mes(dest, SOURCE, channel);
    mes.SetRelativeDist(dist);    
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;        
};

int device_calls::GetRelativeMoveP(GetRelativeMoveParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqRelativeMoveParams,12,GET_MOVERELPARAMS,GetRelativeMoveParams) 
    return 0;
};

int device_calls::SetAbsoluteMoveP(uint32_t pos, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetAbsoluteMoveParams mes(dest, SOURCE, channel);
    mes.SetAbsolutePos(pos);    
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetAbsoluteMoveP(GetAbsoluteMoveParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqAbsoluteMoveParams,12,GET_MOVEABSPARAMS,GetAbsoluteMoveParams) 
    return 0;
};

int device_calls::SetHomingVel(uint32_t vel, int8_t dest,  uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetHomeParams mes(dest, SOURCE, channel);
    if (mes.SetHomingVelocity(vel) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetHomingVel(GetHomeParams *message, uint8_t dest, uint8_t channel){
    GET_MESS(ReqHomeParams,20,GET_HOMEPARAMS,GetHomeParams) 
    return 0;
};

int device_calls::SetLimitSwitchConfig(uint16_t CwHwLim, uint16_t CCwHwLim, uint16_t CwSwLim, uint16_t CCwSwLim, uint16_t mode, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetLimitSwitchParams mes(dest, SOURCE, channel);
    if (mes.SetClockwiseHardLimit(CwHwLim) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetCounterlockwiseHardLimit(CCwHwLim) == INVALID_PARAM) return INVALID_PARAM_2;
    if (mes.SetClockwiseSoftLimit(CwSwLim) == IGNORED_PARAM) printf("Software limit ignored in this device");
    if (mes.SetCounterlockwiseSoftLimit(CCwSwLim)== IGNORED_PARAM) printf("Software limit ignored in this device");
    ret = mes.SetLimitMode(mode);
    if (ret == INVALID_PARAM) return INVALID_PARAM_5;
    if (ret == IGNORED_PARAM) printf("Limit mode ignored in this device");
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetLimitSwitchConfig(GetLimitSwitchParams *message, uint8_t dest, uint8_t channel){
    GET_MESS(ReqLimitSwitchParams,22,GET_LIMSWITCHPARAMS,GetLimitSwitchParams) 
    return 0;
};

int device_calls::MoveToHome(uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveHome mes(dest, SOURCE,channel);        
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].homing=true;  
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::StartSetRelativeMove(uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveRelative1 mes(dest,SOURCE,channel);        
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::StartRelativeMove(int32_t dist, uint8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveRelative2 mes(dest,SOURCE,channel);
    mes.SetRelativeDistance(dist);
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::StartSetAbsoluteMove(uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    MoveAbsolute1 mes(dest,SOURCE,channel);        
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::StartAbsoluteMove(int32_t pos, uint8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE       
    MoveAbsolute2 mes(dest,SOURCE,channel);  
    if (mes.SetAbsoluteDistance(pos) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::StartJogMove(uint8_t direction, uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE       
    JogMove mes(dest,SOURCE,channel);  
    if (mes.SetDirection(direction) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::StartSetVelocityMove(uint8_t direction, uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE        
    MovewVelocity mes(dest,SOURCE,channel);  
    if (mes.SetDirection(direction) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].moving=true;
    EMPTY_IN_QUEUE 
    return 0;
}

int device_calls::StopMovement(uint8_t stopMode, uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    StopMove mes(dest,SOURCE,channel);  
    if (mes.SetStopMode(stopMode) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    opened_device.motor[mes.GetMotorID()].stopping=true;
    EMPTY_IN_QUEUE 
    return 0;
}

int device_calls::SetAccelerationProfile(uint16_t index, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetBowIndex mes(dest, SOURCE, channel);
    if (mes.SetBowindex(index) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetAccelerationProfile(GetBowIndex *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqBowIndex,10,GET_BOWINDEX,GetBowIndex) 
    return 0;
};

int device_calls::SetLedP(uint16_t mode, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetLedMode mes(dest, SOURCE, channel);
    if (mes.SetMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetLedP(GetLedMode *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqLedMode,10,GET_AVMODES,GetLedMode) 
    return 0;
};

int device_calls::SetButtons(uint16_t mode, int32_t pos1, int32_t pos2, uint16_t timeout, int8_t dest, uint16_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetButtonParams mes(dest, SOURCE, channel);
    if (mes.SetMode(mode) == INVALID_PARAM) return INVALID_PARAM_1;
    if (mes.SetPosition1(pos1) == INVALID_PARAM) return INVALID_PARAM_2;
    if (mes.SetPosition2(pos2) == INVALID_PARAM) return INVALID_PARAM_3;
    if (mes.SetTimeout(timeout) == IGNORED_PARAM ) printf("Timeout ignored in this device");
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetButtonsInfo(GetButtonParams *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqButtonParams,22,GET_BUTTONPARAMS,GetButtonParams) 
    return 0;
};

int device_calls::ReqStatus(uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)                
    EMPTY_IN_QUEUE                                          
    ReqStatusUpdate mes(dest, SOURCE, channel);              
    SendMessage(mes);                                       
    EMPTY_IN_QUEUE  
    return 0;
};

int device_calls::ReqDcStatus(uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)                
    EMPTY_IN_QUEUE                                          
    ReqMotChanStatusUpdate mes(dest, SOURCE, channel);              
    SendMessage(mes);                                       
    EMPTY_IN_QUEUE  
    return 0;
};

int device_calls::SendServerAlive(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    ServerAlive mes(dest,SOURCE);
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetStatBits(GetStatusBits *message ,uint8_t dest, uint8_t channel){
    GET_MESS(ReqStatusBits,12,GET_STATUSBITS,GetStatusBits) 
    return 0;
};

int device_calls::DisableEomMessages(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    DisableEndMoveMessages mes(dest,SOURCE);
    SendMessage(mes);
    opened_device.end_of_move_messages = false;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::EnableEomMessages(uint8_t dest){
    CHECK_ADDR_PARAMS(dest, -1)
    EMPTY_IN_QUEUE
    EnableEndMoveMessages mes(dest,SOURCE);
    SendMessage(mes);
    opened_device.end_of_move_messages = true;
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::CreateTrigger(uint8_t mode, uint8_t dest, uint8_t channel){
    CHECK_ADDR_PARAMS(dest, channel)
    EMPTY_IN_QUEUE
    SetTrigger mes(dest,SOURCE, channel);
    if (mes.SetMode(mode) == IGNORED_PARAM) printf("trigger ignored in this device");;
    SendMessage(mes);
    EMPTY_IN_QUEUE 
    return 0;
};

int device_calls::GetMotorTrigger(GetTrigger *message, uint8_t dest, uint8_t channel){
    GET_MESS(ReqTrigger,HEADER_SIZE,GET_TRIGGER,GetTrigger) 
    return 0;
};

int OpenDevice(int index){
    if (index >= devices_connected) return INVALID_PARAM_1;
    if (opened_device_index != -1 ){
        device_calls::StopUpdateMess();
        FT_Close(opened_device.handle);
    }
    opened_device = connected_device[index];
    FT_HANDLE handle;
    FT_STATUS ft_status;
    ft_status = FT_OpenEx( opened_device.SN, FT_OPEN_BY_SERIAL_NUMBER, &handle);
    if (ft_status != FT_OK ) { fprintf(stderr, "Error opening device: %d\n", ft_status); return FT_ERROR; }
    opened_device.handle = handle;
    opened_device_index = index;
    if (ft_status != FT_OK ) { fprintf(stderr, "Error opening device: %d\n", ft_status); return FT_ERROR; }
    if (FT_SetBaudRate(opened_device.handle, 115200) != FT_OK) return FT_ERROR;
    if (FT_SetDataCharacteristics(opened_device.handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE) != FT_OK ) return FT_ERROR;
    usleep(50);
    if (FT_Purge(opened_device.handle, FT_PURGE_RX | FT_PURGE_TX) != FT_OK ) return FT_ERROR;
    usleep(50);
    if (FT_SetFlowControl(opened_device.handle, FT_FLOW_RTS_CTS, 0, 0) != FT_OK) return FT_ERROR;
    if (FT_SetRts(opened_device.handle) != FT_OK ) return FT_ERROR;
    usleep(100);
    
    device_calls::StartUpdateMess();
    return 0;
};

//--------------------------- Input queue monitoring thread -----------------------

int buff_descriptor = 0;

pthread_t buffer_monitor;

void *bufferMonitor(void *){
    int old;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);

    while(true){
        FT_STATUS ftStatus;
        unsigned int bytes;
        ftStatus = FT_GetQueueStatus(opened_device.handle, &bytes);
        if (ftStatus != FT_OK ) {
            fprintf(stderr,"FT_Error occured in monitoring thread, error code: %d\n", ftStatus );
            fprintf(stderr,"Cancelling thread\n");
            return NULL;
        }
        if (bytes > 0){
            raise(SIGUSR1);
        }
        pthread_yield();
    }
}

int getInBuffDesc(){
    return buff_descriptor;
}

int monInit(){
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &sset, NULL) == -1){
        fprintf(stderr,"Failed block signal,error mess: %s\n", strerror(errno));
        return -1;
    }
    buff_descriptor = signalfd(-1, &sset, 0);
    if (buff_descriptor == -1){
        fprintf(stderr,"Failed use descriptor for signal catching, error mess: %s\n", strerror(errno));
        return -1;
    }        
    int ret_stat = pthread_create(&buffer_monitor, NULL, bufferMonitor, NULL);
    if (ret_stat != 0){
       fprintf(stderr,"Failed to create new thread, error mess: %s\n", strerror(errno));
       return -1;
    }
    return 0;
};

void monDestr(){
    int ret_stat = pthread_cancel(buffer_monitor);
    if ( ret_stat != 0 ){
        fprintf(stderr,"Phtread cancel failed, errno: %d\nKilling thread\n", ret_stat);
        ret_stat = pthread_kill(buffer_monitor, SIGKILL);
        if ( ret_stat !=0 ) fprintf(stderr,"Kill failed, errno: %d\n", ret_stat);
    }
    ret_stat = pthread_join(buffer_monitor, NULL);
    if ( ret_stat !=0 ) fprintf(stderr,"Wait for thread failed, errno: %d\n", ret_stat);
    return;
};

