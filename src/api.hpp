#ifndef API
#define API

/*
 * Api for thorlabs messages.
 */
#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ftdi_lib/ftd2xx.h"
#include "device.hpp"
#include "message_codes.hpp"
#define HEADER_SIZE 6

#define INVALID_PARAM -5
#define IGNORED_PARAM -4
#define WARNING -3


class Message{
public:
    Message(uint8_t* buffer, unsigned int buffer_size){
        bytes = (uint8_t *) std::malloc(buffer_size);
        bytes = (uint8_t *) memcpy(bytes, buffer, buffer_size);
        length = buffer_size;
    }
    
    Message(unsigned int in_size){
        bytes = (uint8_t *) malloc(in_size);
        length = in_size;
    }
    
    ~Message(){
        std::free(bytes);
    }
 
    uint8_t* data(){ return bytes; }
    
    unsigned int msize(){ return length; }
    
protected: 
    unsigned int length;
    uint8_t *bytes;
};


class MessageHeader:public Message{
public:
    MessageHeader(uint8_t *header_bytes): Message(header_bytes, HEADER_SIZE){}
    
    MessageHeader(uint16_t type, uint8_t param1, uint8_t param2 ,uint8_t dest, uint8_t source):Message(HEADER_SIZE){
        *((uint16_t *) &bytes[0]) = htole16(type);
        bytes[2] = param1;
        bytes[3] = param2;
        bytes[4] = dest;
        bytes[5] = source;
    }
    
    void GetParams(uint8_t *p1, uint8_t *p2){
        *p1 =(uint8_t) bytes[2];
        *p2 =(uint8_t) bytes[3];
    }
    
    void SetParams(uint8_t p1, uint8_t p2){
        bytes[2] = p1;
        bytes[3] = p2;
    }
    
    void SetFirstParam(uint8_t param){ bytes[2] = param; }
    uint8_t GetFirstParam(){return bytes[2] ;}
    
    void SetSecondParam(uint8_t param){ bytes[3] = param; }
    uint8_t GetSecondParam(){return bytes[3] ;}
    
    uint8_t GetSource(){ return bytes[5]; }
    void SetSource(uint8_t source){ bytes[5] = source; } 

    uint8_t GetDest(){ return bytes[4]; }
    void SetDest(uint8_t dest){
       bytes[4] = dest;
    }
    
    uint16_t GetType(){ return le16toh(*((uint16_t *) &bytes[0])) ;}
    
};

class LongMessage:public Message{
public:
    LongMessage(uint8_t *input_bytes, unsigned int buffer_size):Message(input_bytes, buffer_size){};
    
    LongMessage(uint16_t type, uint16_t data_size, uint8_t dest, uint8_t source):Message(HEADER_SIZE + data_size){
        *((uint16_t *) &bytes[0]) = htole16(type);
        *((uint16_t *) &bytes[2]) = htole16(data_size);
        bytes[4] = dest | 0x80;
        bytes[5] = source;
    }
    
    void SetDest(uint8_t dest){ bytes[4] = (dest | 0x80); }
    uint8_t GetDest(){ return bytes[4]; }

    uint8_t GetSource(){ return bytes[5]; }
    void SetSource(uint8_t source){ bytes[5] = source; } 
    
    uint16_t GetPacketLength(){ return le16toh(*((uint16_t *) &bytes[2]));}
    void SetPacketLength(uint16_t size){ *((uint16_t *) &bytes[2]) = htole16(size);}
    
    uint16_t GetType(){ return le16toh(*((uint16_t *) &bytes[0])) ;}

    uint16_t GetChanID(){ return le16toh(*((uint16_t*) &bytes[6])); }
    
    int SetChanID(int16_t chanID){ 
        if (chanID > opened_device.channels ) return INVALID_PARAM  ;
        *((uint16_t *) &bytes[6]) = htole16(chanID); 
        return 0; 
    }
};


// Generic messages -------------------------------------------------------------

/** Flash front panel LED. */
class IdentifyMs:public MessageHeader{
public:
    IdentifyMs(uint8_t dest, uint8_t source):MessageHeader(IDENTIFY, 0, 0, dest, source){}
};


/** Enable or disable drive channel. */
class SetChannelState:public MessageHeader{
public:
    SetChannelState(uint8_t chanID, uint8_t ableState, uint8_t dest, uint8_t source):MessageHeader(SET_CHANENABLESTATE, chanID, ableState, dest, source){};
    
    /**
     * @brief Set channel id to change state. 
     * @param state - 0x01 for enable, 0x02 for disable 
     */
    int SetAbleState(uint8_t state){
        if (state != 1 && state != 2) return INVALID_PARAM;
        SetSecondParam(state);
        return 0;
    }
};

/** Asks for information about specified channel. */
class ReqChannelState:public MessageHeader{
public:
    ReqChannelState(uint8_t chanID, uint8_t dest, uint8_t source):MessageHeader(REQ_CHANENABLESTATE, chanID, 0, dest, source){};
};

/** Info sent from device. */
class GetChannelState:public MessageHeader{
public:
    GetChannelState(uint8_t *mess):MessageHeader(mess){}
    
    /**
     * @brief Saves info in given variables.
     * @param chanID
     * @param state - 0x01 = enabled, 0x02 = disabled
     */
    void Getinfo(uint8_t *chanID, uint8_t *state){ GetParams(chanID, state); }
};

class HwDisconnect:public MessageHeader{
public:
    HwDisconnect(uint8_t dest, uint8_t source):MessageHeader( HW_DISCONNECT, 0, 0, dest, source){}
    
    HwDisconnect(uint8_t *mess):MessageHeader(mess){}
};

/** Sent from device to notify of unexpected event. */
class HwResponse: public MessageHeader{
public:
    HwResponse(uint8_t *mess):MessageHeader(mess){}
};

/** Sent from device to specify error. */
class HwResponseInfo:public LongMessage{
public:
    HwResponseInfo(uint8_t *mess):LongMessage(mess, 74){}
    
    /**
     * @return ID of message that caused error 
     */
    uint16_t GetMsgID(){ return le16toh(*((uint16_t *) &bytes[6])); }
    
    /**
     * @return Thorlabs specific error code 
     */
    uint16_t GetCode(){ return le16toh(*((uint16_t *) &bytes[8])); }
    
    /**
     * @return  Closer description of error. ASCII string terminated with '\0'.
     */
    char* GetDescription(){
        return (char*) &bytes[10];
    }
};

class StartUpdateMessages:public MessageHeader{
public:
    StartUpdateMessages(uint8_t dest, uint8_t source):MessageHeader(HW_START_UPDATEMSGS, 0, 0, dest, source){};
    
    int SetUpdaterate(uint8_t rate){ 
        if ( opened_device.device_type == BBD101 || opened_device.device_type == BBD102 || opened_device.device_type == BBD103 ) return IGNORED_PARAM;
        SetFirstParam(rate); 
        return 0;
    }
};

class StopUpdateMessages:public MessageHeader{
public:
    StopUpdateMessages(uint8_t dest, uint8_t source):MessageHeader(HW_STOP_UPDATEMSGS, 0, 0, dest, source){}
};

class ReqHwInfo: public MessageHeader{
public:
    ReqHwInfo(uint8_t dest, uint8_t source):MessageHeader(HW_REQ_INFO, 0, 0, dest, source){}
};

class HwInfo:public LongMessage{
public:
    HwInfo(uint8_t *mess):LongMessage(mess, 90){}
    
    int32_t SerialNumber(){ return le32toh(*((int32_t*) &bytes[6])) ; }
    
    std::string ModelNumber(){ 
        std::string ret;
        ret.assign((char*) &bytes[10], 8);
        return ret;
    }
    
    /**
     * @return 44 = brushless DC controller, 45 multi channel motherboard 
     */
    uint16_t HWType(){ return le16toh(*((uint16_t*) &bytes[18])) ;}
    
    std::string Notes(){
        std::string ret;
        ret.assign((char*) &bytes[24], 48);
        return ret;
    }
    
    uint16_t HwVersion(){ return le16toh(*((uint16_t*) &bytes[84])); }
    
    uint16_t ModState(){ return le16toh(*((uint16_t*) &bytes[86])); }
    
    uint16_t NumChannels(){ return le16toh(*((uint16_t*) &bytes[88])); };
};
    
class ReqRackBayUsed:public MessageHeader{
public:
    ReqRackBayUsed( uint8_t dest, uint8_t source):MessageHeader(RACK_REQ_BAYUSED, 0, 0 , dest, source){}

    void SetBayIdent(uint8_t bayID){ SetFirstParam(bayID); }
};

class GetRackBayUsed:public MessageHeader{
public:
    GetRackBayUsed(uint8_t *mess):MessageHeader(mess){}

    uint8_t GetBayID(){ return GetFirstParam(); }

    /**
     * @return Baystates: 0x01 - bay occupied, 0x02 - bay empty 
     */
    uint8_t GetBayState(){ return GetSecondParam(); }
};

class ReqHubBayUsed:public MessageHeader{
public:
    ReqHubBayUsed(uint8_t dest, uint8_t source):MessageHeader(HUB_REQ_BAYUSED, 0, 0, dest, source){}
};

class GetHubBayUsed:public MessageHeader{
public:
    GetHubBayUsed(uint8_t *mess):MessageHeader(mess){}
    
    int8_t GetBayID(){ return GetFirstParam(); }
};


//Motor control messages ---------------------------------------------------

class YesFlashProg:public MessageHeader{
public:
    YesFlashProg(uint8_t dest, uint8_t source):MessageHeader(HW_YES_FLASH_PROGRAMMING, 0, 0, dest, source){};
};

/* Part of initialization. Notifies device of addresses. */
class NoFlashProg: public MessageHeader{
public:
    NoFlashProg(uint8_t dest, uint8_t source):MessageHeader(HW_NO_FLASH_PROGRAMMING, 0, 0, dest, source){};
};


class SetPosCounter:public LongMessage{
public:
    SetPosCounter(uint8_t dest, uint8_t source, uint16_t chanID):LongMessage(SET_POSCOUNTER, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanID);
    }
    
    int SetPosition(int32_t pos){
        if (opened_device.motor[GetChanID()].max_pos < pos) return INVALID_PARAM;
        *((int32_t *) &bytes[8]) = htole32(pos);  
        return WARNING;
    }
};

class ReqPosCounter:public MessageHeader{
public:
    ReqPosCounter(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_POSCOUNTER, chanId, 0, dest, source){}
};

class GetPosCounter:public LongMessage{
public:
    GetPosCounter(uint8_t *mess):LongMessage(mess,12){};

    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
};

class SetEncCount:public LongMessage{
public:
    SetEncCount(uint8_t dest, uint8_t source, uint16_t chanID):LongMessage(SET_ENCCOUNTER, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanID);
    }
    
    int SetEncoderCount(int32_t count){
        if (opened_device.motor[GetChanID()].enc_count == -1 ) return IGNORED_PARAM;
        *((int32_t *) &bytes[8]) = htole32(count); 
        return WARNING;
    }
};

class ReqEncCount:public MessageHeader{
public:
    ReqEncCount(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_ENCCOUNTER, chanId, 0, dest, source){}
};

class GetEncCount:public LongMessage{
public:
    GetEncCount(uint8_t *mess):LongMessage(mess, 12){}

    int32_t GetEncCounter(){ return le32toh(*((int16_t*) &bytes[8])); }
};

class SetVelocityParams:public LongMessage{
public:
    SetVelocityParams(uint8_t dest, uint8_t source, uint16_t chanId )
            :LongMessage(SET_VELPARAMS, 14, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(0);
    };
    
    int SetAcceleration(int32_t acc){
        if( abs(acc) > opened_device.motor[GetChanID()].max_acc ) return INVALID_PARAM;
        *((int32_t *) &bytes[12]) = htole32(acc);
        return 0;
    }
    int SetMaxVel(int32_t max){ 
        if( abs(max) > opened_device.motor[GetChanID()].max_vel ) return INVALID_PARAM;
        *((int32_t *) &bytes[16]) = htole32(max);
        return 0;
    }
};

class ReqVelocityParams:public MessageHeader{
public:
    ReqVelocityParams(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_VELPARAMS, chanId, 0, dest, source){}
};

class GetVelocityParams: public LongMessage{
public:
    GetVelocityParams(uint8_t *mess):LongMessage(mess, 20){}
    
    int32_t GetMinVel(){ return le32toh(*((int32_t*) &bytes[8])); }
    int32_t GetMaxVel(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t GetAcceleration(){ return le32toh(*((int32_t*) &bytes[16])); }
};

class SetJogParams:public LongMessage{
public:
    SetJogParams(uint8_t dest, uint8_t source, uint16_t chanId )
            :LongMessage(SET_JOGPARAMS, 22, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
                *((int32_t *) &bytes[14]) = htole32(0); 
                }
            
    /**
     * @param mode 1 for continuous jogging, 2 for single step
     */
    int SetJogMode(uint16_t mode){
        if(mode != 1 && mode != 2) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(mode); 
        return 0;
    }
    void SetStepSize(int32_t stepSize){ *((int32_t *) &bytes[10]) = htole32(stepSize); }
    
    int SetMaxVelocity(int32_t velocity){
        if( abs(velocity) > opened_device.motor[GetChanID()].max_vel ) return INVALID_PARAM;
        *((int32_t *) &bytes[22]) = htole32(velocity);
        return 0;
    }
    int SetAcceleration(int32_t acc){
        if( abs(acc) > opened_device.motor[GetChanID()].max_acc ) return INVALID_PARAM;
        *((int32_t *) &bytes[18]) = htole32(acc);
        return 0;
    }
    /**
     * @param mode 1 for immediate stop, 2 for profiled stop
     */
    int SetStopMode(uint16_t mode){ 
        if(mode != 1 && mode != 2) return INVALID_PARAM;
        *((uint16_t *) &bytes[26]) = htole16(mode);
        return 0;
    }
            
};

class ReqJogParams:public MessageHeader{
public:
    ReqJogParams(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_JOGPARAMS, chanId, 0, dest, source){}
};

class GetJogParams:public LongMessage{
public:
    GetJogParams(uint8_t *mess):LongMessage(mess, 28){}
    
    /**
     * @return 1 for continuous jogging, 2 for single step 
     */
    uint16_t GetJogMode(){ return le16toh(*((uint16_t*) &bytes[8])); }
    int32_t GetStepSize(){ return le32toh(*((int32_t*) &bytes[10])); }
    int32_t GetMinVel(){ return le32toh(*((int32_t*) &bytes[14])); }
    int32_t GetAcceleration(){ return le32toh(*((int32_t*) &bytes[18])); }
    int32_t GetMaxVel(){ return le32toh(*((int32_t*) &bytes[22])); }
    /** 
     * @return 1 for immediate stop, 2 for profiled stop 
     */
    uint16_t GetStopMode(){ return le16toh(*((uint16_t*) &bytes[26])); }
};

class ReqADCInputs:public MessageHeader{
public:
    ReqADCInputs(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_ADCINPUTS, chanId, 0, dest, source){}
};

class GetADCInputs:public LongMessage{
public:
    GetADCInputs(uint8_t *mess):LongMessage(mess, 10){}
    /**
     * @return values between 0 and 32768, corresponds to 0V and 5V
     */
    uint16_t GetADCInput(){ return le16toh(*((uint16_t*) &bytes[6])); }
};

class SetPowerParams:public LongMessage{
public:
    SetPowerParams(uint8_t dest, uint8_t source, uint16_t chanId )
            :LongMessage(SET_POWERPARAMS, 6, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
    
    /**
     * @param rest_fac, range from 1 to 100, i.e 1% to 100%
     */
    int SetRestFactor(uint16_t rest_fac){ 
        if (rest_fac > 100 || rest_fac == 0) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(rest_fac);
        return 0;
    }
    
    /**
     * @param rest_fac, range from 1 to 100, i.e 1% to 100%
     */
    int SetMoveFactor(uint16_t move_fac){
        if (move_fac > 100 || move_fac == 0) return INVALID_PARAM;
        *((uint16_t *) &bytes[10]) = htole16(move_fac);
        return 0;
    }
};

class ReqPowerParams: public MessageHeader{
public:
    ReqPowerParams(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_POWERPARAMS, chanId, 0, dest, source){}
};

class GetPowerParams:public LongMessage{
public:
    GetPowerParams(uint8_t *mess):LongMessage(mess, 12){}
    
    /**
     * @return phase power when motor is at rest in % 
     */
    uint16_t GetRestFactor(){ return le16toh(*((uint16_t*) &bytes[8]));  }
    /**
     * @return phase power when motor is moving in % 
     */
    uint16_t GetMoveFactor(){ return le16toh(*((uint16_t*) &bytes[10]));  }
};

class SetGeneralMoveParams:public LongMessage{
    SetGeneralMoveParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_GENMOVEPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    void SetBacklashDist(int32_t dist){ *((int32_t *) &bytes[8]) = htole16(dist); }
};

class ReqGeneralMoveParams:public MessageHeader{
public:
    ReqGeneralMoveParams(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_GENMOVEPARAMS, chanId, 0, dest, source){}
};

class GetGeneralMoveParams:public LongMessage{
public:
    GetGeneralMoveParams(uint8_t *mess):LongMessage(mess,12){};
    
    int32_t GetBakclashDist(){return le32toh(*((int32_t*) &bytes[8])); }
};

class SetRelativeMoveParams:public LongMessage{
public:
    SetRelativeMoveParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_MOVERELPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        }

    void SetRelativeDist(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

class ReqRelativeMoveParams:public MessageHeader{
public:
    ReqRelativeMoveParams(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_MOVERELPARAMS, chanId, 0, dest, source){}
};

class GetRelativeMoveParams: public LongMessage{
    GetRelativeMoveParams(uint8_t *mess):LongMessage(mess,6){}
    
    int32_t GetRelativeDist(){ return le32toh(*((int32_t*) &bytes[8])); }
};

class SetAbsoluteMoveParams:public LongMessage{
public: 
    SetAbsoluteMoveParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_MOVEABSPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);   
        }
            
    int SetAbsolutePos(int32_t pos){ 
        if (pos < 0 || pos > opened_device.motor[GetChanID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[8]) = htole32(pos); 
        return 0;
    }
};

class ReqAbsoluteMoveParams:public MessageHeader{
public:
    ReqAbsoluteMoveParams(uint8_t dest, uint8_t source, uint8_t chanId): MessageHeader(REQ_MOVEABSPARAMS, chanId, 0, dest, source){}
};

class GetAbsoluteMoveParams:public LongMessage{
public:
    GetAbsoluteMoveParams(uint8_t *mess):LongMessage(mess,12){}
    
    int32_t GetAbsolutePos(){ return le32toh(*((int32_t*) &bytes[8])); }
};

class SetHomeParams:public LongMessage{
public:
    SetHomeParams(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_HOMEPARAMS, 14, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    int SetHomingVelocity(int32_t vel){
        if (vel < 0 || vel > opened_device.motor[GetChanID()].max_vel ) return INVALID_PARAM;
        *((int32_t *) &bytes[12]) = htole32(vel); 
        return 0;
    }
};

class ReqHomeParams:public MessageHeader{
public:
    ReqHomeParams(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_HOMEPARAMS, chanId, 0, dest, source){}
};

class GetHomeParams:public LongMessage{
public:
    GetHomeParams(uint8_t *mess):LongMessage(mess,20){}
    
    int32_t GetHomingVelocity(){ return le32toh(*((int32_t*) &bytes[12])); }
};

class SetLimitSwitchParams:public LongMessage{
public:
    SetLimitSwitchParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_LIMSWITCHPARAMS, 16, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
    
    int SetClockwiseHardLimit(uint16_t limit){
        if ( limit != 0x80 && limit > 0x06 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(limit);
        return 0;
    }
    int SetCounterlockwiseHardLimit(uint16_t limit){ 
        if ( limit != 0x80 && limit > 0x06 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[10]) = htole16(limit);
        return 0;
    }
    
    int SetClockwiseSoftLimit(int32_t limit){
        if (opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        *((int32_t *) &bytes[12]) = htole32(limit); 
        return 0;
    }
    int SetCounterlockwiseSoftLimit(int32_t limit){ 
        if (opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        *((int32_t *) &bytes[16]) = htole32(limit); 
        return 0;
    }  
    
    int SetLimitMode(uint16_t mode){ 
        if ( opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        if ( mode != 0x80 && mode > 0x03 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[20]) = htole16(mode); 
        return 0;
    }
};

class ReqLimitSwitchParams:public MessageHeader{
public:
    ReqLimitSwitchParams(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_LIMSWITCHPARAMS, chanId, 0, dest, source){}
};

class GetLimitSwitchParams:public LongMessage{
public:
    GetLimitSwitchParams(uint8_t *mess):LongMessage(mess, 22){}
    
    uint16_t GetClockwiseHardLimit(){ return le16toh(*((uint16_t*) &bytes[8])); }
    uint16_t GetCounterlockwiseHardLimit(){ return le16toh(*((uint16_t*) &bytes[10])); }
    
    int32_t SetClockwiseSoftLimit(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t SetCounterlockwiseSoftLimit(){ return le32toh(*((int32_t*) &bytes[16])); }
    
    uint16_t GetLimitMode(){ return le16toh(*((uint16_t*) &bytes[20])); }
};

class MoveHome:public MessageHeader{
public:
    MoveHome(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(MOVE_HOME, chanId, 0, dest, source){}
};

class MovedHome:public MessageHeader{
public:
    MovedHome(uint8_t *mess):MessageHeader(mess){}
};

class MoveRelative1:public MessageHeader{
public:
    MoveRelative1(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(MOVE_RELATIVE, chanId, 0, dest, source){}
};

class MoveRelative2:public LongMessage{
public:
    MoveRelative2(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(MOVE_RELATIVE, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    void SetRelativeDistance(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

class MoveCompleted:public MessageHeader{
public:
    MoveCompleted(uint8_t *mess):MessageHeader(mess){};
};


class MoveAbsolute1:public MessageHeader{
public:
    MoveAbsolute1(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(MOVE_ABSOLUTE, chanId, 0 , dest, source){}
};

class MoveAbsolute2:public LongMessage{
public:
    MoveAbsolute2(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(MOVE_ABSOLUTE, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    };
     
    int SetAbsoluteDistance(int32_t dist){
        if (dist < 0 || dist > opened_device.motor[GetChanID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[8]) = htole32(dist); 
        return 0;
    }
};

class JogMove:public MessageHeader{
public:
    JogMove(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(MOVE_JOG, chanId, 1, dest, source){}
    
    int SetDirection(uint8_t direction){
        if (direction != 0x01 && direction != 0x02) return INVALID_PARAM;
        SetSecondParam(direction);
        return 0;
    }
};

class MovewVelocity:public MessageHeader{
public:
    MovewVelocity(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(MOVE_VELOCITY,chanId, 1, dest, source){}
    
    int SetDirection(uint8_t direction){
        if (direction != 0x01 && direction != 0x02) return INVALID_PARAM;
        SetSecondParam(direction);
        return 0;
    }
};

class StopMove:public MessageHeader{
public:
    StopMove(uint8_t dest, uint8_t source, uint8_t chanId ):MessageHeader(MOVE_STOP, chanId, 2, dest, source){}
    
    /**
     * @param mode, 1 for immediate stop, 2 for profiled stop
     */
    int SetStopMode(uint8_t mode){
        if (mode != 0x01 && mode != 0x02) return INVALID_PARAM;
        SetSecondParam(mode);
        return 0;
    }
};

class MoveStopped: public MessageHeader{
public:
    MoveStopped(uint8_t *mess):MessageHeader(mess){}
}; 

class SetBowIndex:public LongMessage{
public:
    SetBowIndex(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_BOWINDEX, 4, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    /**
     * @param profile of acceleration/deceleration, 0 for trapezoidal, 1-18 for s-curve profile
     */
    int SetBowindex(uint16_t index){
        if ( index > 18 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(index); 
        return 0;
    }
};

class ReqBowIndex:public MessageHeader{
public:
    ReqBowIndex(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_BOWINDEX, chanId, 0, dest, source){}
};

class GetBowIndex:public LongMessage{
public:
    GetBowIndex(uint8_t *mess):LongMessage(mess, 10){};
    
    uint16_t BowIndex(){ return le16toh(*((uint16_t*) &bytes[8])); }
};

class SetPidParams:public LongMessage{
public:
    SetPidParams(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_DCPIDPARAMS, 20 , dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
            
    int SetProportional(int32_t value){ 
        if ( value < 0 || value > 32767 ) return INVALID_PARAM;
        *((int32_t *) &bytes[8]) = htole32(value); 
        return 0;
    }
    
    int SetIntegeral(int32_t value){ 
        if ( value < 0 || value > 32767 ) return INVALID_PARAM;
        *((int32_t *) &bytes[12]) = htole32(value); 
        return 0;
    }
    
    int SetDifferential(int32_t value){ 
        if ( value < 0 || value > 32767 ) return INVALID_PARAM;
        *((int32_t *) &bytes[16]) = htole32(value);
        return 0;
    }
    
    int SetIntegralLimit(int32_t value){ 
        if ( value < 0 || value > 32767 ) return INVALID_PARAM;
        *((int32_t *) &bytes[20]) = htole32(value); 
        return 0;
    }
    
    void SetFilterControl(uint16_t control){ *((uint16_t *) &bytes[24]) = htole16(control); }
};

class ReqPidParams:public MessageHeader{
public:
    ReqPidParams(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_DCPIDPARAMS, chanId, 0, dest, source){}
};

class GetPidParams:public LongMessage{
public:
    GetPidParams(uint8_t *mess):LongMessage(mess, 26){}

    int32_t GetProportional(){ return le32toh(*((int32_t*) &bytes[8])); }
    int32_t GetIntegral(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t GetDifferential(){ return le32toh(*((int32_t*) &bytes[16])); }
    int32_t GetIntegralLimit(){ return le32toh(*((int32_t*) &bytes[20])); }
    
    uint16_t GetFilterControl(){ return le16toh(*((uint16_t*) &bytes[24])); }
};

class SetLedMode:public LongMessage{
public:
    SetLedMode(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_AVMODES, 4, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    int SetMode(uint16_t mode){ 
        if ( mode > 11 || ( mode > 3 || mode < 8) ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(mode); 
         return 0;
    }
       
};

class ReqLedMode:public MessageHeader{
public:
    ReqLedMode(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_AVMODES, chanId, 0, dest, source){}
};

class GetLedMode:public LongMessage{
public:
    GetLedMode(uint8_t *mess):LongMessage(mess, 10){}
    
    uint16_t GetMode(){ return le16toh(*((uint16_t*) &bytes[8])); } 
};

class SetButtonParams:public LongMessage{
public:
    SetButtonParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_BUTTONPARAMS, 16, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
            
    int SetMode(uint16_t mode){
        if (mode != 1 && mode != 2 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(mode); 
        return 0;
    }
    int SetPosition1(int32_t pos){
        if (pos < 0 || pos > opened_device.motor[GetChanID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[10]) = htole32(pos);
        return 0;
    }
    int SetPosition2(int32_t pos){
        if (pos < 0 || pos > opened_device.motor[GetChanID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[14]) = htole32(pos);
        return 0;
    }
    int SetTimeout(uint16_t ms){ 
        if (opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        *((uint16_t *) &bytes[18]) = htole16(ms); 
        return 0;
    }
};

class ReqButtonParams:public MessageHeader{
public:
    ReqButtonParams(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_BUTTONPARAMS, chanId, 0, dest, source){}
};

class GetButtonParams: public LongMessage{
public:
    GetButtonParams(uint8_t *mess):LongMessage(mess, 22){}
    
    uint16_t GetMode(){ return le16toh(*((uint16_t*) &bytes[8])); }
    int32_t GetPosition1(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t GetPosition2(){ return le32toh(*((int32_t*) &bytes[14])); }
    uint16_t GetTimeout(){ return le16toh(*((uint16_t*) &bytes[18])); }
};

class ReqStatusUpdate:public MessageHeader{
public: 
    ReqStatusUpdate(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_STATUSUPDATE, chanId, 0, dest, source){}
};

class GetStatusUpdate:public LongMessage{
public:
    GetStatusUpdate(uint8_t *mess):LongMessage(mess,20){}
    
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
    int32_t GetEncCount(){ return le32toh(*((int32_t*) &bytes[12])); }
    uint32_t GetStatusBits(){ return le32toh(*((uint32_t*) &bytes[16])); }
};

class ReqMotChanStatusUpdate:public MessageHeader{
public:
    ReqMotChanStatusUpdate(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_DCSTATUSUPDATE, chanId, 0, dest, source){}
};

class GetMotChanStatusUpdate:public LongMessage{
public:
    GetMotChanStatusUpdate(uint8_t *mess):LongMessage(mess,20){}
    
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
    uint16_t GetVelocity(){ return le16toh(*((uint16_t*) &bytes[12])); }
    uint32_t GetStatusBits(){ return le32toh(*((uint32_t*) &bytes[16])); }
};

class ServerAlive:public MessageHeader{
public:
    ServerAlive(uint8_t dest, uint8_t source):MessageHeader(ACK_DCSTATUSUPDATE, 0, 0, dest, source){}
};

class ReqStatusBits:public MessageHeader{
public:
    ReqStatusBits(uint8_t dest, uint8_t source,  uint8_t chanId):MessageHeader(REQ_STATUSBITS, chanId, 0, dest, source){}
};

class GetStatusBits:public LongMessage{
public:
    GetStatusBits(uint8_t *mess):LongMessage(mess,12){}
    
    uint32_t GetStatBits(){ return le32toh(*((uint32_t*) &bytes[8])); }
};

class DisableEndMoveMessages:public MessageHeader{
public:
    DisableEndMoveMessages(uint8_t dest, uint8_t source):MessageHeader(SUSPEND_ENDOFMOVEMSGS, 0, 0, dest, source){}
};

class EnableEndMoveMessages:public MessageHeader{
public:
    EnableEndMoveMessages(uint8_t dest, uint8_t source):MessageHeader(RESUME_ENDOFMOVEMSGS, 0, 0, dest, source){}
};

class SetTrigger:public MessageHeader{
public:
    SetTrigger(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(SET_TRIGGER, chanId, 0, dest, source){}
    
    int SetMode(uint8_t mode){
        if (opened_device.device_type != BSC201 && opened_device.device_type != BSC202 && opened_device.device_type != BSC203 && 
            opened_device.device_type != TBD001 && opened_device.device_type != BBD201 && opened_device.device_type != BBD202 && 
            opened_device.device_type != BBD203) 
            return IGNORED_PARAM;
        SetSecondParam(mode);        
        return 0;        
    }
    
};

class ReqTrigger:public MessageHeader{
public:
    ReqTrigger(uint8_t dest, uint8_t source, uint8_t chanId):MessageHeader(REQ_TRIGGER, chanId, 0, dest, source){}
};

class GetTrigger:public MessageHeader{
public:
    GetTrigger(uint8_t *mess):MessageHeader(mess){}
    
    uint8_t GetMode(){return GetSecondParam() ;}
};

 #endif 