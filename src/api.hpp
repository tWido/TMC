/*
 * Api for thorlabs messages.
 */
#include "../ftdi_lib/ftd2xx.h"
#include "message_codes.hpp"
#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define HEADER_SIZE 6
#define GET_CH_ID_FUNC uint16_t GetChanID(){ return le16toh(*((uint16_t*) &bytes[6])); }
#define SET_CH_ID_FUNC void SetChanID(uint16_t chanID){*((uint16_t *) &bytes[6]) = htole16(chanID); }


class Message{
public:
    Message(uint8_t* buffer, unsigned int buffer_size){
        bytes = (uint8_t *) malloc(buffer_size);
        bytes = (uint8_t *) memcpy(bytes, buffer, buffer_size);
        length = buffer_size;
    }
    
    Message(unsigned int in_size){
        bytes = (uint8_t *) malloc(in_size);
        length = in_size;
    }
    
    ~Message(){
        free(bytes);
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
        bytes[4] = dest;
        bytes[5] = source;
    }
    
    void SetDest(uint8_t dest){ bytes[4] = (dest | 0x80); }
    uint8_t GetDest(){ return bytes[4]; }

    uint8_t GetSource(){ return bytes[5]; }
    void SetSource(uint8_t source){ bytes[5] = source; } 
    
    uint16_t GetPacketLength(){ return le16toh(*((uint16_t *) &bytes[2]));}
    void SetPacketLength(uint16_t size){ *((uint16_t *) &bytes[2]) = htole16(size);}
    
    uint16_t GetType(){ return le16toh(*((uint16_t *) &bytes[0])) ;}

};


// Generic messages -------------------------------------------------------------

/** Flash front panel LED. */
class IdentifyMs:public MessageHeader{
public:
    IdentifyMs(uint8_t dest = 0x50, uint8_t source = 0x01 ):MessageHeader(IDENTIFY, 0, 0, dest, source){}
};


/** Enable or disable drive channel. */
class SetChannelState:public MessageHeader{
public:
    SetChannelState(uint8_t chanID = 0x01, uint8_t ableState = 0 , uint8_t dest = 0x50, uint8_t source = 0x01 ):MessageHeader(SET_CHANENABLESTATE, chanID, ableState, dest, source){};
    
    /**
     * @brief Set channel id to change state. 
     * @param chanID - starting at 0x01
     */
    void SetChannelIdent(uint8_t chanID){ SetFirstParam(chanID); }
    
    /**
     * @brief Set channel id to change state. 
     * @param state - 0x01 for enable, 0x02 for disable 
     */
    void SetAbleState(uint8_t state){ SetSecondParam(state); }
};

/** Asks for information about specified channel. */
class ReqChannelState:public MessageHeader{
public:
    ReqChannelState(uint8_t chanID = 0x01, uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(REQ_CHANENABLESTATE, chanID, 0, dest, source){};
   
    /** 
     * @brief Set channel id which info is required. 
     * @param chanID - starting at 0x01
     */
    void SetChannelIdent(uint8_t chanID){ SetFirstParam(chanID); }
};

/** Info sent from device. */
class ChannelStateInfo:public MessageHeader{
public:
    ChannelStateInfo(uint8_t *mess):MessageHeader(mess){}
    
    /**
     * @brief Saves info in given variables.
     * @param chanID
     * @param state - 0x01 = enabled, 0x02 = disabled
     */
    void Getinfo(uint8_t *chanID, uint8_t *state){ GetParams(chanID, state); }
};

class HwDisconnect:public MessageHeader{
public:
    HwDisconnect(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader( HW_DISCONNECT, 0, 0, dest, source){}
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
    StartUpdateMessages(uint8_t rate = 0, uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(HW_START_UPDATEMSGS, rate, 0, dest, source){};
    
    void SetUpdaterate(uint8_t rate){ SetFirstParam(rate); }
};

class StopUpdateMessages:public MessageHeader{
public:
    StopUpdateMessages(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(HW_STOP_UPDATEMSGS, 0, 0, dest, source){}
};

class ReqHwInfo: public MessageHeader{
public:
    ReqHwInfo(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(HW_REQ_INFO, 0, 0, dest, source){}
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
    ReqRackBayUsed(uint8_t bayID, uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(RACK_REQ_BAYUSED, bayID, 0 , dest, source){}

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
    ReqHubBayUsed(uint8_t dest = 0x50, uint8_t source = 0x01 ):MessageHeader(HUB_REQ_BAYUSED, 0, 0, dest, source){}
};

class GetBayUsed:public MessageHeader{
public:
    GetBayUsed(uint8_t *mess):MessageHeader(mess){}

    uint8_t GetBayID(){ return GetFirstParam(); }
};


//Motor control messages ---------------------------------------------------

class YesFlashProg:public MessageHeader{
public:
    YesFlashProg(uint8_t dest =  0x50, uint8_t source = 0x01):MessageHeader(HW_YES_FLASH_PROGRAMMING, 0, 0, dest, source){};
};

/* Part of initialization. Notifies device of addresses. */
class NoFlashProg: public MessageHeader{
public:
    NoFlashProg(uint8_t dest =  0x50, uint8_t source = 0x01):MessageHeader(HW_NO_FLASH_PROGRAMMING, 0, 0, dest, source){};
};


class SetPosCounter:public LongMessage{
public:
    SetPosCounter(uint8_t dest =  0x50, uint8_t source = 0x01, uint16_t chanID  = 1, int32_t pos = 0 ):LongMessage(SET_POSCOUNTER, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanID);
        *((uint32_t *) &bytes[8]) = htole32(pos);
    }
    SET_CH_ID_FUNC
    void SetPosition(int32_t pos){ *((int32_t *) &bytes[8]) = htole32(pos); }
};

class ReqPosCounter:public MessageHeader{
public:
    ReqPosCounter(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_POSCOUNTER, chanId, 0, dest, source){}
};

class GetPosCounter:public LongMessage{
public:
    GetPosCounter(uint8_t *mess):LongMessage(mess,12){};

    GET_CH_ID_FUNC
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
};

class SetEncCount:public LongMessage{
public:
    SetEncCount(uint8_t dest =  0x50, uint8_t source = 0x01, uint16_t chanID  = 1, int32_t enc_count = 0):LongMessage(SET_ENCCOUNTER, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanID);
        *((int32_t *) &bytes[8]) = htole32(enc_count);
    }
};

class ReqEncCount:public MessageHeader{
public:
    ReqEncCount(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_ENCCOUNTER, chanId, 0, dest, source){}
};

class GetEncCount:public LongMessage{
public:
    GetEncCount(uint8_t *mess):LongMessage(mess, 12){}

    GET_CH_ID_FUNC
    int32_t GetEncCounter(){ return le32toh(*((int16_t*) &bytes[8])); }

};

class SetVelocityParams:public LongMessage{
public:
    SetVelocityParams(uint8_t dest =  0x50, uint8_t source = 0x01, uint16_t chanId = 1, int32_t min_vel = 0, int32_t acc = 0, int32_t max_vel = 0 )
            :LongMessage(SET_VELPARAMS, 14, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(min_vel);
        *((int32_t *) &bytes[12]) = htole32(acc);
        *((int32_t *) &bytes[16]) = htole32(max_vel);
    };
    
    SET_CH_ID_FUNC
    void SetMinVel(int32_t min){ *((int32_t *) &bytes[8]) = htole32(min); }
    void SetMaxVel(int32_t max){ *((int32_t *) &bytes[12]) = htole32(max); }
    void SetAcceleration(int32_t acc){ *((int32_t *) &bytes[16]) = htole32(acc); }
};

class ReqVelocityParams:public MessageHeader{
public:
    ReqVelocityParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_VELPARAMS, chanId, 0, dest, source){}
};

class GetVelocityParams: public LongMessage{
public:
    GetVelocityParams(uint8_t *mess):LongMessage(mess, 14){}
    
    GET_CH_ID_FUNC
    int32_t GetMinVel(){ return le32toh(*((int32_t*) &bytes[8])); }
    int32_t GetMaxVel(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t GetAcceleration(){ return le32toh(*((int32_t*) &bytes[16])); }
};

class SetJogParams:public LongMessage{
public:
    SetJogParams(uint8_t dest =  0x50, uint8_t source = 0x01, uint16_t chanId = 1, uint16_t mode =1, int32_t stepSize = 0, 
            int32_t minVel = 0, int32_t acc = 0, int32_t maxVel = 0, uint16_t stopMode = 2)
            :LongMessage(SET_JOGPARAMS, 22, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((uint16_t *) &bytes[8]) = htole16(mode);
        *((int32_t *) &bytes[10]) = htole32(stepSize);
        *((int32_t *) &bytes[14]) = htole32(minVel);
        *((int32_t *) &bytes[18]) = htole32(acc);
        *((int32_t *) &bytes[22]) = htole32(maxVel);
        *((uint16_t *) &bytes[24]) = htole16(stopMode);       
        }
            
    SET_CH_ID_FUNC
    /**
     * @param mode 1 for continuous jogging, 2 for single step
     */
    void SetJogMode(uint16_t mode){ *((uint16_t *) &bytes[8]) = htole16(mode); }
    void SetStepSize(int32_t stepSize){ *((int32_t *) &bytes[10]) = htole32(stepSize); }
    void SetMinVelocity(int32_t velocity){ *((int32_t *) &bytes[14]) = htole32(velocity); }
    void SetMaxVelocity(int32_t velocity){ *((int32_t *) &bytes[22]) = htole32(velocity); }
    void SetAcceleration(int32_t acc){ *((int32_t *) &bytes[18]) = htole32(acc); }
    /**
     * @param mode 1 for immediate stop, 2 for profiled stop
     */
    void SetStopMode(uint16_t mode){ *((uint16_t *) &bytes[26]) = htole16(mode); }
            
};

class ReqJogParams:public MessageHeader{
public:
    ReqJogParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_JOGPARAMS, chanId, 0, dest, source){}
};

class GetJogParams:public LongMessage{
public:
    GetJogParams(uint8_t *mess):LongMessage(mess, 28){}
    
    GET_CH_ID_FUNC
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
    uint16_t GetStopMode(){ return le16toh(*((uint16_t*) &bytes[126])); }
};

class ReqADCInputs:public MessageHeader{
public:
    ReqADCInputs(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_ADCINPUTS, chanId, 0, dest, source){}
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
    SetPowerParams(uint8_t dest = 0x50, uint8_t source = 0x01, uint16_t chanId = 1, uint16_t RestFactor = 0, uint16_t MoveFactor = 0 )
            :LongMessage(SET_POWERPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((uint16_t *) &bytes[8]) = htole16(RestFactor);
        *((uint16_t *) &bytes[10]) = htole16(MoveFactor);
            }
    
    SET_CH_ID_FUNC
    void SetRestFactor(uint16_t rest_fac){ *((uint16_t *) &bytes[8]) = htole16(rest_fac); }
    void SetMoveFactor(uint16_t move_fac){ *((uint16_t *) &bytes[10]) = htole16(move_fac); }
};

class ReqPowerParams: public MessageHeader{
public:
    ReqPowerParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_POWERPARAMS, chanId, 0, dest, source){}
};

class GetPowerParams:public LongMessage{
public:
    GetPowerParams(uint8_t *mess):LongMessage(mess, 12){}
    
    GET_CH_ID_FUNC
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
    SetGeneralMoveParams(uint8_t dest = 0x50,uint8_t source = 0x01, uint16_t chanId = 1, int32_t BacklashDist = 0 )
            :LongMessage(SET_GENMOVEPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(BacklashDist);
    }
    
    SET_CH_ID_FUNC
    void SetBacklashDist(int32_t dist){ *((int32_t *) &bytes[8]) = htole16(dist); }
};

class ReqGeneralMoveParams:public MessageHeader{
public:
    ReqGeneralMoveParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_GENMOVEPARAMS, chanId, 0, dest, source){}
};

class GetGeneralMoveParams:public LongMessage{
public:
    GetGeneralMoveParams(uint8_t *mess):LongMessage(mess,12){};
    
    GET_CH_ID_FUNC
    int32_t GetBakclashDist(){return le32toh(*((int32_t*) &bytes[8])); }
};

class SetRelativeMoveParams:public LongMessage{
public:
    SetRelativeMoveParams(uint8_t dest = 0x50,uint8_t source = 0x01, uint16_t chanId = 1, int32_t RelativeDist = 0)
            :LongMessage(SET_MOVERELPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(RelativeDist);
        }
            
    SET_CH_ID_FUNC
    void SetRelativeDist(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

class ReqRelativeMoveParams:public MessageHeader{
public:
    ReqRelativeMoveParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_MOVERELPARAMS, chanId, 0, dest, source){}
};

class GetRelativeMoveParams: public LongMessage{
    GetRelativeMoveParams(uint8_t *mess):LongMessage(mess,6){}
    
    GET_CH_ID_FUNC
    int32_t GetRelativeDist(){ return le32toh(*((int32_t*) &bytes[8])); }
};

class SetAbsoluteMoveParams:public LongMessage{
public: 
    SetAbsoluteMoveParams(uint8_t dest = 0x50,uint8_t source = 0x01, uint16_t chanId = 1, int32_t AbsolutePos = 0)
            :LongMessage(SET_MOVEABSPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(AbsolutePos);    
        }
            
    SET_CH_ID_FUNC
    void SetAbsolutePos(int32_t pos){ *((int32_t *) &bytes[8]) = htole32(pos); }
};

class ReqAbsoluteMoveParams:public MessageHeader{
public:
    ReqAbsoluteMoveParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1): MessageHeader(REQ_MOVEABSPARAMS, chanId, 0, dest, source){}
};

class GetAbsoluteMoveParams:public LongMessage{
public:
    GetAbsoluteMoveParams(uint8_t *mess):LongMessage(mess,12){}
    
    GET_CH_ID_FUNC
    int32_t GetAbsolutePos(){ return le32toh(*((int32_t*) &bytes[8])); }
};

class SetHomeParams:public LongMessage{
public:
    SetHomeParams(uint8_t dest = 0x50,uint8_t source = 0x01, uint16_t chanId = 1, int32_t HomingVel = 1):LongMessage(SET_HOMEPARAMS, 14, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[12]) = htole32(HomingVel); 
    }
    
    SET_CH_ID_FUNC
    void SetHomingVelocity(int32_t vel){ *((int32_t *) &bytes[12]) = htole32(vel); }
};

class ReqHomeParams:public MessageHeader{
public:
    ReqHomeParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_HOMEPARAMS, chanId, 0, dest, source){}
};

class GetHomeParams:public LongMessage{
public:
    GetHomeParams(uint8_t *mess):LongMessage(mess,20){}
    
    GET_CH_ID_FUNC
    int32_t GetHomingVelocity(){ return le32toh(*((int32_t*) &bytes[12])); }
};

class SetLimitSwitchParams:public LongMessage{
public:
    SetLimitSwitchParams(uint8_t dest = 0x50,uint8_t source = 0x01, uint16_t chanId = 1, uint16_t CWHardLimit = 0x01, uint16_t CCWHardLimit = 0x01,
            int32_t CWSoftLimit = 0x01, int32_t CCWSoftLimit = 0x01, uint16_t LimitMode = 0x01 )
            :LongMessage(SET_LIMSWITCHPARAMS, 16, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
                *((uint16_t *) &bytes[8]) = htole16(CWHardLimit);
                *((uint16_t *) &bytes[10]) = htole16(CCWHardLimit);
                *((int32_t *) &bytes[12]) = htole32(CWSoftLimit);
                *((int32_t *) &bytes[16]) = htole32(CCWSoftLimit);
                *((uint16_t *) &bytes[20]) = htole16(LimitMode);
            }
            
    SET_CH_ID_FUNC
    
    void SetClockwiseHardLimit(uint16_t limit){ *((uint16_t *) &bytes[8]) = htole16(limit); }
    void SetCounterlockwiseHardLimit(uint16_t limit){ *((uint16_t *) &bytes[10]) = htole16(limit); }
    
    void SetClockwiseSoftLimit(int32_t limit){ *((int32_t *) &bytes[12]) = htole32(limit); }
    void SetCounterlockwiseSoftLimit(int32_t limit){ *((int32_t *) &bytes[16]) = htole32(limit); }  
    
    void SetLimitMode(uint16_t mode){ *((uint16_t *) &bytes[20]) = htole16(mode); }
};

class ReqLimitSwitchParams:public MessageHeader{
public:
    ReqLimitSwitchParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_LIMSWITCHPARAMS, chanId, 0, dest, source){}
};

class GetLimitSwitchParams:public LongMessage{
public:
    GetLimitSwitchParams(uint8_t *mess):LongMessage(mess, 22){}
    
    GET_CH_ID_FUNC
    uint16_t GetClockwiseHardLimit(){ return le16toh(*((uint16_t*) &bytes[8])); }
    uint16_t GetCounterlockwiseHardLimit(){ return le16toh(*((uint16_t*) &bytes[10])); }
    
    int32_t SetClockwiseSoftLimit(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t SetCounterlockwiseSoftLimit(){ return le32toh(*((int32_t*) &bytes[16])); }
    
    uint16_t GetLimitMode(){ return le16toh(*((uint16_t*) &bytes[20])); }
};

class MoveHome:public MessageHeader{
public:
    MoveHome(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(MOVE_HOME, chanId, 0, dest, source){}
};

class MovedHome:public MessageHeader{
public:
    MovedHome(uint8_t *mess):MessageHeader(mess){}
};

class MoveRelative1:public MessageHeader{
public:
    MoveRelative1(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(MOVE_RELATIVE, chanId, 0, dest, source){}
    
    
};

class MoveRelative2:public LongMessage{
public:
    MoveRelative2(uint8_t dest =  0x50, uint8_t source = 0x01,  uint16_t chanId = 1, int32_t RelativeDist = 0):LongMessage(MOVE_RELATIVE, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((uint32_t *) &bytes[8]) = htole32(RelativeDist);
    }
    
    SET_CH_ID_FUNC
    void SetRelativeDistance(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

class MoveCompleted:public MessageHeader{
public:
    MoveCompleted(uint8_t *mess):MessageHeader(mess){};
};


class MoveAbsolute1:public MessageHeader{
public:
    MoveAbsolute1(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(MOVE_ABSOLUTE, chanId, 0 , dest, source){}
};

class MoveAbsolute2:public LongMessage{
public:
    MoveAbsolute2(uint8_t dest =  0x50, uint8_t source = 0x01,  uint16_t chanId = 1, int32_t distance = 0):LongMessage(MOVE_ABSOLUTE, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(distance);
    };
     
    SET_CH_ID_FUNC
    void SetAbsoluteDistance(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

class JogMove:public MessageHeader{
public:
    JogMove(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1, uint8_t direction = 1):MessageHeader(MOVE_JOG, chanId, direction, dest, source){}
};

class MovewVelocity:public MessageHeader{
public:
    MovewVelocity(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1, uint8_t direction = 1):MessageHeader(MOVE_VELOCITY,chanId, direction, dest, source){}
};

class StopMove:public MessageHeader{
public:
    StopMove(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1, uint8_t StopMode = 2):MessageHeader(MOVE_STOP, chanId, StopMode, dest, source){}
};

class MoveStopped: public MessageHeader{
public:
    MoveStopped(uint8_t *mess):MessageHeader(mess){}
}; 

class SetBowIndex:public LongMessage{
public:
    SetBowIndex(uint8_t dest =  0x50, uint8_t source = 0x01,  uint16_t chanId = 1, uint16_t bowIndex = 2):LongMessage(SET_BOWINDEX, 4, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((uint16_t *) &bytes[8]) = htole16(bowIndex);
    }
    
    SET_CH_ID_FUNC
    /**
     * @param profile of acceleration/deceleration, 0 for trapezoidal, 1-18 for s-curve profile
     */
    void SetBowindex(uint16_t index){ *((uint16_t *) &bytes[8]) = htole16(index); }
};

class ReqBowIndex:public MessageHeader{
public:
    ReqBowIndex(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_BOWINDEX, chanId, 0, dest, source){}
};

class GetBowIndex:public LongMessage{
public:
    GetBowIndex(uint8_t *mess):LongMessage(mess, 10){};
    
    GET_CH_ID_FUNC
    uint16_t BowIndex(){ return le16toh(*((uint16_t*) &bytes[8])); }
};

class SetPidParams:public LongMessage{
public:
    SetPidParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint16_t chanId = 1, int32_t proportional = 0, int32_t integral = 0, 
            int32_t differential = 0, int32_t integralLimit = 0, uint16_t FilterControl = 0):LongMessage(SET_DCPIDPARAMS, 20 , dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
                *((int32_t *) &bytes[8]) = htole32(proportional);
                *((int32_t *) &bytes[12]) = htole32(integral);
                *((int32_t *) &bytes[16]) = htole32(differential);
                *((int32_t *) &bytes[20]) = htole32(integralLimit);
                *((uint16_t *) &bytes[24]) = htole16(FilterControl);
            }
            
    SET_CH_ID_FUNC
    void SetProportional(int32_t value){ *((int32_t *) &bytes[8]) = htole32(value); }
    void SetIntegeral(int32_t value){ *((int32_t *) &bytes[12]) = htole32(value); }
    void SetDifferential(int32_t value){ *((int32_t *) &bytes[16]) = htole32(value); }
    void SetIntegralLimit(int32_t value){ *((int32_t *) &bytes[20]) = htole32(value); }
    
    void SetFilterControl(uint16_t control){ *((uint16_t *) &bytes[24]) = htole16(control); }
};

class ReqPidParams:public MessageHeader{
public:
    ReqPidParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_DCPIDPARAMS, chanId, 0, dest, source){}
};

class GetPidParams:public LongMessage{
public:
    GetPidParams(uint8_t *mess):LongMessage(mess, 26){}
    
    GET_CH_ID_FUNC
    int32_t GetProportional(){ return le32toh(*((int32_t*) &bytes[8])); }
    int32_t GetIntegral(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t GetDifferential(){ return le32toh(*((int32_t*) &bytes[16])); }
    int32_t GetIntegralLimit(){ return le32toh(*((int32_t*) &bytes[20])); }
    
    uint16_t GetFilterControl(){ return le16toh(*((uint16_t*) &bytes[24])); }
};

class SetLedMode:public LongMessage{
public:
    SetLedMode(uint8_t dest =  0x50, uint8_t source = 0x01,  uint16_t chanId = 1, uint16_t mode = 1):LongMessage(SET_AVMODES, 4, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((uint16_t *) &bytes[8]) = htole16(mode);
    }
    SET_CH_ID_FUNC
    void SetMode(uint16_t mode){ *((uint16_t *) &bytes[8]) = htole16(mode); }
};

class ReqLedMode:public MessageHeader{
public:
    ReqLedMode(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_AVMODES, chanId, 0, dest, source){}
};

class GetLedMode:public LongMessage{
public:
    GetLedMode(uint8_t *mess):LongMessage(mess, 10){}
    
    GET_CH_ID_FUNC
    uint16_t GetMode(){ return le16toh(*((uint16_t*) &bytes[8])); } 
};

class SetButtonParams:public LongMessage{
public:
    SetButtonParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint16_t chanId = 1, uint16_t mode = 1, int32_t pos1 = 0 , int32_t pos2 = 0, uint16_t timeout= 500)
            :LongMessage(SET_BUTTONPARAMS, 16, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
                *((uint16_t *) &bytes[8]) = htole16(mode);
                *((int32_t *) &bytes[10]) = htole32(pos1);
                *((int32_t *) &bytes[14]) = htole32(pos2);
                *((uint16_t *) &bytes[18]) = htole16(timeout);
            }
            
    SET_CH_ID_FUNC
    void SetMode(uint16_t mode){ *((uint16_t *) &bytes[8]) = htole16(mode); }
    void SetPosition1(int32_t pos){ *((int32_t *) &bytes[10]) = htole32(pos); }
    void SetPosition2(int32_t pos){ *((int32_t *) &bytes[14]) = htole32(pos); }
    void SetTimeout(uint16_t ms){ *((uint16_t *) &bytes[18]) = htole16(ms); }
};

class ReqButtonParams:public MessageHeader{
public:
    ReqButtonParams(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_BUTTONPARAMS, chanId, 0, dest, source){}
};

class GetButtonParams: public LongMessage{
public:
    GetButtonParams(uint8_t *mess):LongMessage(mess, 22){}
    
    GET_CH_ID_FUNC
    uint16_t GetMode(){ return le16toh(*((uint16_t*) &bytes[8])); }
    int32_t GetPosition1(){ return le32toh(*((int32_t*) &bytes[12])); }
    int32_t GetPosition2(){ return le32toh(*((int32_t*) &bytes[14])); }
    uint16_t GetTimeout(){ return le16toh(*((uint16_t*) &bytes[18])); }
};

class SetActuatorType:public MessageHeader{
public:
    SetActuatorType(uint8_t dest =  0x50, uint8_t source = 0x01,  uint8_t actuatorId = 1):MessageHeader(SET_TSTACTUATORTYPE, actuatorId, 0, dest, source){}
    
    void SetActuatorId(uint8_t id){ SetFirstParam(id) ;}
};

class ReqStatusUpdate:public MessageHeader{
public: 
    ReqStatusUpdate(uint8_t dest = 0x50, uint8_t source = 0x01,  uint8_t chanId = 1 ):MessageHeader(REQ_STATUSUPDATE, chanId, 0, dest, source){}
};

class GetStatusUpdate:public LongMessage{
public:
    GetStatusUpdate(uint8_t *mess):LongMessage(mess,20){}
    
    GET_CH_ID_FUNC
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
    int32_t GetEncCount(){ return le32toh(*((int32_t*) &bytes[12])); }
    uint32_t GetStatusBits(){ return le32toh(*((uint32_t*) &bytes[16])); }
};

class ReqMotChanStatusUpdate:public MessageHeader{
public:
    ReqMotChanStatusUpdate(uint8_t dest = 0x50, uint8_t source = 0x01,  uint8_t chanId = 1 ):MessageHeader(REQ_DCSTATUSUPDATE, chanId, 0, dest, source){}
};

class GetMotChanStatusUpdate:public LongMessage{
public:
    GetMotChanStatusUpdate(uint8_t *mess):LongMessage(mess,20){}
    
    GET_CH_ID_FUNC
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
    uint16_t GetVelocity(){ return le16toh(*((uint16_t*) &bytes[12])); }
    uint32_t GetStatusBits(){ return le32toh(*((uint32_t*) &bytes[16])); }
};

class ServerAlive:public MessageHeader{
public:
    ServerAlive(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(ACK_DCSTATUSUPDATE, 0, 0, dest, source){}
};

class ReqStatusBits:public MessageHeader{
public:
    ReqStatusBits(uint8_t dest = 0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_STATUSBITS, chanId, 0, dest, source){}
};

class GetStatusBits:public LongMessage{
public:
    GetStatusBits(uint8_t *mess):LongMessage(mess,12){}
    
    GET_CH_ID_FUNC
    uint32_t GetStatBits(){ return le32toh(*((uint32_t*) &bytes[8])); }
};

class DisableEndMoveMessages:public MessageHeader{
public:
    DisableEndMoveMessages(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(SUSPEND_ENDOFMOVEMSGS, 0, 0, dest, source){}
};

class EnableEndMoveMessages:public MessageHeader{
public:
    EnableEndMoveMessages(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(RESUME_ENDOFMOVEMSGS, 0, 0, dest, source){}
};

class SetTrigger:public MessageHeader{
public:
    SetTrigger(uint8_t dest = 0x50, uint8_t source = 0x01,  uint8_t chanId = 1, uint8_t mode = 0):MessageHeader(SET_TRIGGER, chanId, mode, dest, source){}
};

class ReqTrigger:public MessageHeader{
public:
    ReqTrigger(uint8_t dest = 0x50, uint8_t source = 0x01,  uint8_t chanId = 1):MessageHeader(REQ_TRIGGER, chanId, 0, dest, source){}
};

class GetTrigger:public MessageHeader{
public:
    GetTrigger(uint8_t *mess):MessageHeader(mess){}
    
    uint8_t GetMode(){return GetSecondParam() ;}
};