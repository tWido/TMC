#ifndef MESSAGES
#define MESSAGES

#include <endian.h>
#include <string>
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

/*
 * Base class for messages.
 */
class Message{
public:
    /**
     * Basic constructor. Allocates required space and copies buffer to message.
     * @param buffer- buffer pointer
     * @param buffer_size  - buffer size
     */
    Message(uint8_t* buffer, unsigned int buffer_size){
        bytes = (uint8_t *) malloc(buffer_size);
        bytes = (uint8_t *) memcpy(bytes, buffer, buffer_size);
        length = buffer_size;
    }
    
    /**
     * Basic constructor. Allocates required space.
     * @param in_size - size to allocate
     */
    Message(unsigned int in_size){
        bytes = (uint8_t *) malloc(in_size);
        length = in_size;
    }
    
    /**
     * Frees allocated memory.
     */
    ~Message(){
        free(bytes);
    }
    
    /**
     * Returns pointer to message data.
     * @return uint8_t pointer to start of message data buffer
     */
    uint8_t* data(){ return bytes; }
    
    /**
     * Returns size of message data.
     * @return buffer size
     */
    unsigned int msize(){ return length; }
    
protected: 
    unsigned int length;
    uint8_t *bytes;
};

/*
 * Message composed only of header and no additional data.
 */
class HeaderMessage:public Message{
public:
    /**
     * Constructs empty message composed of header only.
     */
    HeaderMessage():Message(6){}
    
    /**
     * Constructs message and copies data buffer to message. 
     * @param header_bytes - pointer to buffer to copy
     */
    HeaderMessage(uint8_t *header_bytes): Message(header_bytes, HEADER_SIZE){}
    
    /**
     * Construct message with specified parameters.
     * @param type - number specifying message type
     * @param param1 - first parameter
     * @param param2 -second parameter
     * @param dest - destination address
     * @param source - source address
     */
    HeaderMessage(uint16_t type, uint8_t param1, uint8_t param2 ,uint8_t dest, uint8_t source):Message(HEADER_SIZE){
        *((uint16_t *) &bytes[0]) = htole16(type);
        bytes[2] = param1;
        bytes[3] = param2;
        bytes[4] = dest;
        bytes[5] = source;
    }
    
    /**
     * Sets message data.
     * @param data - pointer to buffer to copy
     */
    void SetData(uint8_t *data){ bytes = (uint8_t *) memcpy(bytes, data, length); }
    
    /**
     * Gets parameters from message.
     * @param p1 - first parameter
     * @param p2 - second parameter
     */
    void GetParams(uint8_t *p1, uint8_t *p2){
        *p1 =(uint8_t) bytes[2];
        *p2 =(uint8_t) bytes[3];
    }
    
    /**
     * Sets message parameters.
     * @param p1 - first parameter
     * @param p2 - second parameter
     */
    void SetParams(uint8_t p1, uint8_t p2){
        bytes[2] = p1;
        bytes[3] = p2;
    }
    
    /**
     * Sets first parameter
     * @param param
     */
    void SetFirstParam(uint8_t param){ bytes[2] = param; }
    /**
     * Returns first parameter
     * @return parameter
     */
    uint8_t GetFirstParam(){return bytes[2] ;}
    
    /**
     * Sets second parameter
     * @param param
     */
    void SetSecondParam(uint8_t param){ bytes[3] = param; }
    /**
     * Returns second parameter
     * @param param
     */
    uint8_t GetSecondParam(){return bytes[3] ;}
    
    /**
     * Returns source address.
     */
    uint8_t GetSource(){ return bytes[5]; }
    /**
     * Sets source address.
     * @param source
     */
    void SetSource(uint8_t source){ bytes[5] = source; } 

    /**
     * Returns destination address.
     */
    uint8_t GetDest(){ return bytes[4]; }
    /**
     * Sets destination address.
     * @param dest
     */
    void SetDest(uint8_t dest){
       bytes[4] = dest;
    }
    
    /**
     * One controller device can be connected up to 3 motors. Function calculates 
     * index to array of connected motors. 
     * @return index in range 0-2
     */
    uint8_t GetMotorID(){
        if (opened_device.bays == -1) return (GetFirstParam()-1);    // channel type, channels numbered from 1, in field from 0
        else {                                                  // bay type, numbering 0x21 ..0x23, 
            if (GetSource() == 0x01) return (GetDest() - 0x21); // outgoing message
            else return (GetSource() - 0x21);                   // incoming message      
        }              
    }
    
    /**
     * @return number specifying message type 
     */
    uint16_t GetType(){ return le16toh(*((uint16_t *) &bytes[0])) ;}
};

/*
 * Message with additional data.
 */
class LongMessage:public Message{
public:
    /**
     * Creates empty message with specified size
     * @param size
     */
    LongMessage(unsigned int size):Message(size){}
    
    /**
     * Creates message, allocates space and copies given buffer to message.
     * @param input_bytes - buffer pointer
     * @param buffer_size - buffer size
     */
    LongMessage(uint8_t *input_bytes, unsigned int buffer_size):Message(input_bytes, buffer_size){};
    
    /**
     * Creates message and sets parameters in header.
     * @param type - number specifying message type
     * @param data_size - size of data following header
     * @param dest - destination address
     * @param source - source address
     */
    LongMessage(uint16_t type, uint16_t data_size, uint8_t dest, uint8_t source):Message(HEADER_SIZE + data_size){
        *((uint16_t *) &bytes[0]) = htole16(type);
        *((uint16_t *) &bytes[2]) = htole16(data_size);
        bytes[4] = dest | 0x80;
        bytes[5] = source;
    }
    
    /**
     * Sets message data.
     * @param data - pointer to buffer to copy
     */
    void SetData(uint8_t *data){ bytes = (uint8_t *) memcpy(bytes, data, length); }
    
    
    /**
     * Sets destination address.
     */
    void SetDest(uint8_t dest){ bytes[4] = (dest | 0x80); }
    /**
     * Returns destination address.
     */
    uint8_t GetDest(){ return bytes[4]; }
    
    /**
     * Returns source address.
     */
    uint8_t GetSource(){ return bytes[5]; }
    /**
     * Sets source address.
     */
    void SetSource(uint8_t source){ bytes[5] = source; } 
    
    /**
     * Return length of data following header.
     * @return length
     */
    uint16_t GetPacketLength(){ return le16toh(*((uint16_t *) &bytes[2]));}
    /**
     * Sets parameter specifying length of data following header.
     * @param size
     */
    void SetPacketLength(uint16_t size){ *((uint16_t *) &bytes[2]) = htole16(size);}
    
    /**
     * Returns number specifying message type.
     * @return type
     */
    uint16_t GetType(){ return le16toh(*((uint16_t *) &bytes[0])) ;}

    /**
     * Return channel ID
     * @return channel ID in range 0x21-0x23
     */
    uint16_t GetChanID(){ return le16toh(*((uint16_t*) &bytes[6])); }
    
    /**
     * Sets channel ID.
     * @param chanID 
     * @return INVALID PARAM when setting invalid ID, 0 on success
     */
    int SetChanID(int16_t chanID){ 
        if (chanID > opened_device.channels ) return INVALID_PARAM  ;
        *((uint16_t *) &bytes[6]) = htole16(chanID); 
        return 0; 
    }
    
    /**
     * One controller device can be connected up to 3 motors. Function calculates 
     * index to array of connected motors. 
     * @return index in range 0-2
     */
    uint16_t GetMotorID(){
        if (opened_device.bays == -1) return (GetChanID()-1);    // channel type, channels numbered from 1, in field from 0
        else {                                                  // bay type, numbering 0x21 ..0x23, 
            if (GetSource() == 0x01) return (GetDest() - 0x21); // outgoing message
            else return (GetSource() - 0x21);                   // incoming message      
        }   
    }
};


// Generic messages -------------------------------------------------------------

/**
 * Flash front panel LED.
 * @param dest - destination address 
 * @param source - source address
 */
class IdentifyMs:public HeaderMessage{
public:
    IdentifyMs(uint8_t dest, uint8_t source):HeaderMessage(IDENTIFY, 0, 0, dest, source){}
};


/**
 * Used to enable or disable channel.
 * @param chanID - channel ID
 * @param ableState - 1 for enable, 2 for disable
 * @param dest - destination address
 * @param source - source address
 */
class SetChannelState:public HeaderMessage{
public:
    SetChannelState(uint8_t chanID, uint8_t ableState, uint8_t dest, uint8_t source):HeaderMessage(SET_CHANENABLESTATE, chanID, ableState, dest, source){};
    
    /**
     * Set channel ability. 
     * @param state - 0x01 for enable, 0x02 for disable 
     * @return INVALID PARAM when setting invalid ID, 0 on success
     */
    int SetAbleState(uint8_t state){
        if (state != 1 && state != 2) return INVALID_PARAM;
        SetSecondParam(state);
        return 0;
    }
};

/**
 * Request message for information about specified channel. 
 * @param chanID - channel ID
 * @param dest - destination address
 * @param source - source address
 */
class ReqChannelState:public HeaderMessage{
public:
    ReqChannelState(uint8_t chanID, uint8_t dest, uint8_t source):HeaderMessage(REQ_CHANENABLESTATE, chanID, 0, dest, source){};
};

/** 
 * Channel info sent from device. 
 */
class GetChannelState:public HeaderMessage{
public:
    GetChannelState():HeaderMessage(){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetChannelState(uint8_t *mess):HeaderMessage(mess){}
    
    /**
     * Saves info in given variables.
     * @param chanID - channel ID
     * @param state - 0x01 = enabled, 0x02 = disabled
     */
    void Getinfo(uint8_t *chanID, uint8_t *state){ GetParams(chanID, state); }
};

/**
 * Sent by host or device to signal disconnect.
 */
class HwDisconnect:public HeaderMessage{
public:
    HwDisconnect():HeaderMessage(){}
    
    /**
     * Construct message and fills addresses
     * @param dest - destination address
     * @param source - source address
     */
    HwDisconnect(uint8_t dest, uint8_t source):HeaderMessage( HW_DISCONNECT, 0, 0, dest, source){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    HwDisconnect(uint8_t *mess):HeaderMessage(mess){}
};

/**
 *  Sent from device to notify of unexpected event. 
 */
class HwResponse: public HeaderMessage{
public:
    HwResponse():HeaderMessage(){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    HwResponse(uint8_t *mess):HeaderMessage(mess){}
};

/** 
 * Sent from device to specify error. 
 */
class HwResponseInfo:public LongMessage{
public:
    HwResponseInfo():LongMessage(74){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
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

/**
 * Sent to device to start sending update messages at specified rate per second. 
 * @param dest - destination address
 * @param source - source address
 */
class StartUpdateMessages:public HeaderMessage{
public:
    StartUpdateMessages(uint8_t dest, uint8_t source):HeaderMessage(HW_START_UPDATEMSGS, 0, 0, dest, source){};
    
    /**
     * Sets update message rate.
     * @param rate - messages sent per second
     * @return IGNORED_PARAM if parameter is ignored in controller device, 0 on success
     */
    int SetUpdaterate(uint8_t rate){ 
        if ( opened_device.device_type == BBD101 || opened_device.device_type == BBD102 || opened_device.device_type == BBD103 ) return IGNORED_PARAM;
        SetFirstParam(rate); 
        return 0;
    }
};

/**
 * Sent to device to stop sending update messages. 
 * @param dest - destination address
 * @param source - source address
 */
class StopUpdateMessages:public HeaderMessage{
public:
    StopUpdateMessages(uint8_t dest, uint8_t source):HeaderMessage(HW_STOP_UPDATEMSGS, 0, 0, dest, source){}
};

/**
 * Request message for device hardware information.
 * @param dest - destination address
 * @param source - source address
 */
class ReqHwInfo: public HeaderMessage{
public:
    ReqHwInfo(uint8_t dest, uint8_t source):HeaderMessage(HW_REQ_INFO, 0, 0, dest, source){}
};

/**
 * Message containing hardware info.
 */
class HwInfo:public LongMessage{
public:
    HwInfo():LongMessage(90){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    HwInfo(uint8_t *mess):LongMessage(mess, 90){}
    
    /**
     * Returns serial number.
     * @return serial number
     */
    int32_t SerialNumber(){ return le32toh(*((int32_t*) &bytes[6])) ; }
    
    /**
     * Returns alphanumeric string describing model type.
     * @return model name in string
     */
    std::string ModelNumber(){ 
        std::string ret;
        ret.assign((char*) &bytes[10], 8);
        return ret;
    }
    
    /**
     * @return 44 = brushless DC controller, 45 = multi channel motherboard 
     */
    uint16_t HWType(){ return le16toh(*((uint16_t*) &bytes[18])) ;}
    
    /**
     * Returns closer description.
     * @return description string
     */
    std::string Notes(){
        std::string ret;
        ret.assign((char*) &bytes[24], 48);
        return ret;
    }
    
    /**
     * @brief Returns hardware version number.
     */
    uint16_t HwVersion(){ return le16toh(*((uint16_t*) &bytes[84])); }
    
    /**
     * @brief Returns modification state of device.
     */
    uint16_t ModState(){ return le16toh(*((uint16_t*) &bytes[86])); }
    
    /**
     * @brief Returns number of channels.
     */
    uint16_t NumChannels(){ return le16toh(*((uint16_t*) &bytes[88])); };
};

/**
 * Requests bay occupation info.
 * @param dest - destination address
 * @param source - source address
 */
class ReqRackBayUsed:public HeaderMessage{
public:
    ReqRackBayUsed( uint8_t dest, uint8_t source):HeaderMessage(RACK_REQ_BAYUSED, 0, 0 , dest, source){}
    
    /**
     * Sets bay ID to request info on.
     * @param bayID
     */
    void SetBayIdent(uint8_t bayID){ SetFirstParam(bayID); }
};

/**
 * Bay occupation info.
 */
class GetRackBayUsed:public HeaderMessage{
public:
    GetRackBayUsed():HeaderMessage(){};
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetRackBayUsed(uint8_t *mess):HeaderMessage(mess){}
    
    /**
     * Return bay ID from message.
     * @return bay ID
     */
    uint8_t GetBayID(){ return GetFirstParam(); }

    /**
     * @return 1 - bay occupied, 2 - bay empty 
     */
    uint8_t GetBayState(){ return GetSecondParam(); }
};


//Motor control messages ---------------------------------------------------

/**
 * Sent on startup. Nu function.
 * @param dest - destination address
 * @param source - source address
 */
class YesFlashProg:public HeaderMessage{
public:
    YesFlashProg(uint8_t dest, uint8_t source):HeaderMessage(HW_YES_FLASH_PROGRAMMING, 0, 0, dest, source){};
};

/**
 * Part of initialization. Notifies device of addresses. 
 * @param dest - destination address
 * @param source - source address
 */
class NoFlashProg: public HeaderMessage{
public:
    NoFlashProg(uint8_t dest, uint8_t source):HeaderMessage(HW_NO_FLASH_PROGRAMMING, 0, 0, dest, source){};
};

/**
 * Sets position counter. Normally unused.
 * @param dest - destination address
 * @param source - source address
 * @param chanID - channel ID
 */
class SetPosCounter:public LongMessage{
public:
    SetPosCounter(uint8_t dest, uint8_t source, uint16_t chanID):LongMessage(SET_POSCOUNTER, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanID);
    }
    
    /**
     * Sets position counter.
     * @param pos - position to set to counter
     * @return WARNING, proceed with caution
     */
    int SetPosition(int32_t pos){
        *((int32_t *) &bytes[8]) = htole32(pos);  
        return WARNING;
    }
};

/**
 * Requests position counter.
 * @param dest - destination address
 * @param source - source address
 * @param chanID - channel ID
 */
class ReqPosCounter:public HeaderMessage{
public:
    ReqPosCounter(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_POSCOUNTER, chanId, 0, dest, source){}
};

/**
 * Contains position counter value.
 */
class GetPosCounter:public LongMessage{
public:
    GetPosCounter():LongMessage(12){};
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetPosCounter(uint8_t *mess):LongMessage(mess,12){};

    /**
     * @return position counter value
     */
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
};

/**
 * Sets encoder counter. Normally unused.
 * @param dest - destination address
 * @param source - source address
 * @param chanID - channel ID
 */
class SetEncCount:public LongMessage{
public:
    SetEncCount(uint8_t dest, uint8_t source, uint16_t chanID):LongMessage(SET_ENCCOUNTER, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanID);
    }
    
     /**
     * Sets encoder counter.
     * @param pos - position to set to counter
     * @return WARNING, proceed with caution
     */
    int SetEncoderCount(int32_t count){
        *((int32_t *) &bytes[8]) = htole32(count); 
        return WARNING;
    }
};

/**
 * Requests encoder counter.
 * @param dest - destination address
 * @param source - source address
 * @param chanID - channel ID
 */
class ReqEncCount:public HeaderMessage{
public:
    ReqEncCount(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_ENCCOUNTER, chanId, 0, dest, source){}
};

/**
 * Contains encoder counter value.
 */
class GetEncCount:public LongMessage{
public:
    GetEncCount():LongMessage(12){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetEncCount(uint8_t *mess):LongMessage(mess, 12){}

    /**
     * @return encoder counter value
     */
    int32_t GetEncCounter(){ return le32toh(*((int16_t*) &bytes[8])); }
};

/**
 * Sets maximal velocity and acceleration.
 */
class SetVelocityParams:public LongMessage{
public:
    /**
     * Construct message and fills addresses
     * @param dest - destination address
     * @param source - source address
     * @param chanId - channel ID
     */
    SetVelocityParams(uint8_t dest, uint8_t source, uint16_t chanId )
            :LongMessage(SET_VELPARAMS, 14, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        *((int32_t *) &bytes[8]) = htole32(0);
    };
    
    /**
     * Sets acceleration in Thorlabs specified units.
     * @param acc - acceleration
     * @return INVALID_PARAM when value exceed restriction, 0 on success
     */
    int SetAcceleration(int32_t acc){
        if( abs(acc) > opened_device.motor[GetMotorID()].max_acc ) return INVALID_PARAM;
        *((int32_t *) &bytes[12]) = htole32(acc);
        return 0;
    }
    
    /**
     * Sets maximal velocity in Thorlabs specified units.
     * @param max - maximal velocity
     * @return INVALID_PARAM when value exceed restriction, 0 on success
     */
    int SetMaxVel(int32_t max){ 
        if( abs(max) > opened_device.motor[GetMotorID()].max_vel ) return INVALID_PARAM;
        *((int32_t *) &bytes[16]) = htole32(max);
        return 0;
    }
};

/**
 * Requests velocity parameters.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqVelocityParams:public HeaderMessage{
public:
    ReqVelocityParams(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_VELPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains maximal velocity and acceleration.
 */
class GetVelocityParams: public LongMessage{
public:
    GetVelocityParams():LongMessage(20){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetVelocityParams(uint8_t *mess):LongMessage(mess, 20){}
    
    /**
     * @return minimal velocity in Thorlabs specified units.
     */
    int32_t GetMinVel(){ return le32toh(*((int32_t*) &bytes[8])); }
    /**
     * @return maximal velocity in Thorlabs specified units.
     */
    int32_t GetMaxVel(){ return le32toh(*((int32_t*) &bytes[16])); }
    /**
     * @return acceleration in Thorlabs specified units.
     */
    int32_t GetAcceleration(){ return le32toh(*((int32_t*) &bytes[12])); }
};

/**
 * Sets parameters of jog move.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetJogParams:public LongMessage{
public:
    SetJogParams(uint8_t dest, uint8_t source, uint16_t chanId )
            :LongMessage(SET_JOGPARAMS, 22, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
                *((int32_t *) &bytes[14]) = htole32(0); 
                }
            
    /**
     * Sets mode.
     * @param mode 1 for continuous jogging, 2 for single step
     */
    int SetJogMode(uint16_t mode){
        if(mode != 1 && mode != 2) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(mode); 
        return 0;
    }
    
    /**
     * Sets size of step if single step mode is used.
     * @param stepSize - size of step in Thorlabs specified units
     */
    void SetStepSize(int32_t stepSize){ *((int32_t *) &bytes[10]) = htole32(stepSize); }
    
    /**
     * Sets maximal velocity.
     * @param velocity - maximal velocity in Thorlabs specified units
     * @return INVALID_PARAM when value exceed restriction, 0 on success
     */
    int SetMaxVelocity(int32_t velocity){
        if( abs(velocity) > opened_device.motor[GetMotorID()].max_vel ) return INVALID_PARAM;
        *((int32_t *) &bytes[22]) = htole32(velocity);
        return 0;
    }
    /**
     * Sets acceleration.
     * @param acc - acceleration in Thorlabs specified units
     * @return INVALID_PARAM when value exceed restriction, 0 on success
     */
    int SetAcceleration(int32_t acc){
        if( abs(acc) > opened_device.motor[GetMotorID()].max_acc ) return INVALID_PARAM;
        *((int32_t *) &bytes[18]) = htole32(acc);
        return 0;
    }
    /**
     * Sets immediate or profiled stop.
     * @param mode 1 for immediate stop, 2 for profiled stop
     */
    int SetStopMode(uint16_t mode){ 
        if(mode != 1 && mode != 2) return INVALID_PARAM;
        *((uint16_t *) &bytes[26]) = htole16(mode);
        return 0;
    }
            
};

/**
 * Requests parameters of jog move.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqJogParams:public HeaderMessage{
public:
    ReqJogParams(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_JOGPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains parameters of jog move.
 */
class GetJogParams:public LongMessage{
public:
    GetJogParams():LongMessage(28){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetJogParams(uint8_t *mess):LongMessage(mess, 28){}
    
    /**
     * @return 1 for continuous jogging, 2 for single step 
     */
    uint16_t GetJogMode(){ return le16toh(*((uint16_t*) &bytes[8])); }
    
    /**
     * @return size of step in Thorlabs specified units
     */
    int32_t GetStepSize(){ return le32toh(*((int32_t*) &bytes[10])); }
    
    /**
     * @return minimal velocity in Thorlabs specified units
     */
    int32_t GetMinVel(){ return le32toh(*((int32_t*) &bytes[14])); }
    
    /**
     * @return acceleration in Thorlabs specified units 
     */
    int32_t GetAcceleration(){ return le32toh(*((int32_t*) &bytes[18])); }
    
    /**
     * @return maximal velocity in Thorlabs specified units 
     */
    int32_t GetMaxVel(){ return le32toh(*((int32_t*) &bytes[22])); }
    
    /** 
     * @return 1 means immediate stop, 2 means profiled stop 
     */
    uint16_t GetStopMode(){ return le16toh(*((uint16_t*) &bytes[26])); }
};

/**
 * Sets power percentage used while moving and while resting.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetPowerParams:public LongMessage{
public:
    SetPowerParams(uint8_t dest, uint8_t source, uint16_t chanId )
            :LongMessage(SET_POWERPARAMS, 6, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
    
    /**
     * Sets power used when motor is resting.
     * @param rest factor - range from 1 to 100, i.e 1% to 100%
     */
    int SetRestFactor(uint16_t rest_fac){ 
        if (rest_fac > 100 || rest_fac == 0) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(rest_fac);
        return 0;
    }
    
    /**
     *  Sets power used when motor is moving.
     * @param move factor - range from 1 to 100, i.e 1% to 100%
     */
    int SetMoveFactor(uint16_t move_fac){
        if (move_fac > 100 || move_fac == 0) return INVALID_PARAM;
        *((uint16_t *) &bytes[10]) = htole16(move_fac);
        return 0;
    }
};

/**
 * Requests power percentage used while moving and while resting.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqPowerParams: public HeaderMessage{
public:
    ReqPowerParams(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_POWERPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains power percentage used while moving and while resting.
 */
class GetPowerParams:public LongMessage{
public:
    GetPowerParams():LongMessage(12){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetPowerParams(uint8_t *mess):LongMessage(mess, 12){}
    
    /**
     * @return phase power used while motor is resting in % 
     */
    uint16_t GetRestFactor(){ return le16toh(*((uint16_t*) &bytes[8]));  }
    /**
     * @return phase power used while motor is moving in % 
     */
    uint16_t GetMoveFactor(){ return le16toh(*((uint16_t*) &bytes[10]));  }
};

/**
 * Sets general move parameters, which consists only of backlash distance.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetGeneralMoveParams:public LongMessage{
public:
    SetGeneralMoveParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_GENMOVEPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    /**
     * Sets backlash distance
     * @param dist - distance in Thorlabs specified units
     */
    void SetBacklashDist(int32_t dist){ *((int32_t *) &bytes[8]) = htole16(dist); }
};

/**
 * Request general move parameter, which consists only of backlash distance.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqGeneralMoveParams:public HeaderMessage{
public:
    ReqGeneralMoveParams(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_GENMOVEPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains general move parameter, which consists only of backlash distance.
 */
class GetGeneralMoveParams:public LongMessage{
public:
    GetGeneralMoveParams():LongMessage(12){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetGeneralMoveParams(uint8_t *mess):LongMessage(mess,12){};
    
    /**
     * @return backlash distance in Thorlabs specified units
     */
    int32_t GetBacklashDist(){return le32toh(*((int32_t*) &bytes[8])); }
};

/**
 * Sets relative distance to move after after calling relative move.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetRelativeMoveParams:public LongMessage{
public:
    SetRelativeMoveParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_MOVERELPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
        }

    void SetRelativeDist(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

/**
 * Requires relative distance set in controller, which is used after calling relative move. 
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqRelativeMoveParams:public HeaderMessage{
public:
    ReqRelativeMoveParams(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_MOVERELPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains relative distance set in controller, which is used after calling relative move. 
 */
class GetRelativeMoveParams: public LongMessage{
public:
    GetRelativeMoveParams():LongMessage(12){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetRelativeMoveParams(uint8_t *mess):LongMessage(mess,12){}
    
    /**
     * @return distance in Thorlabs specified units
     */
    int32_t GetRelativeDist(){ return le32toh(*((int32_t*) &bytes[8])); }
};

/**
 * Sets absolute position to move to after calling absolute move.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetAbsoluteMoveParams:public LongMessage{
public: 
    SetAbsoluteMoveParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_MOVEABSPARAMS, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);   
        }
    
    /**
     * Sets absolute position to move to after absolute move.
     * @param pos - position in THorlabs specified units
     * @return INVALID_PARAM when value exceed restriction, 0 on success
     */        
    int SetAbsolutePos(int32_t pos){ 
        if (pos < 0 || pos > opened_device.motor[GetMotorID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[8]) = htole32(pos); 
        return 0;
    }
};

/**
 * Request absolute position set in controller. Value is used when calling absolute move.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqAbsoluteMoveParams:public HeaderMessage{
public:
    ReqAbsoluteMoveParams(uint8_t dest, uint8_t source, uint8_t chanId): HeaderMessage(REQ_MOVEABSPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains absolute position set in controller. Value is used when calling absolute move.
 */
class GetAbsoluteMoveParams:public LongMessage{
public:
    GetAbsoluteMoveParams():LongMessage(12){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetAbsoluteMoveParams(uint8_t *mess):LongMessage(mess,12){}
    
    /**
     * Returns absolute position set in device, which is used when calling absolute move.
     * @return position in Thorlabs specified units
     */
    int32_t GetAbsolutePos(){ return le32toh(*((int32_t*) &bytes[8])); }
};

/**
 * Sets homing velocity.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetHomeParams:public LongMessage{
public:
    SetHomeParams(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_HOMEPARAMS, 14, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    /**
     * Sets homing velocity for specified motor.
     * @param vel - velocity in Thorlabs specified units
     * @return INVALID_PARAM when value exceed restriction, 0 on success
     */
    int SetHomingVelocity(int32_t vel){
        if (vel < 0 || vel > opened_device.motor[GetMotorID()].max_vel ) return INVALID_PARAM;
        *((int32_t *) &bytes[12]) = htole32(vel); 
        return 0;
    }
};

/**
 * Requests homing velocity to specified motor.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqHomeParams:public HeaderMessage{
public:
    ReqHomeParams(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_HOMEPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains homing velocity for specified motor.
 */
class GetHomeParams:public LongMessage{
public:
    GetHomeParams():LongMessage(20){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetHomeParams(uint8_t *mess):LongMessage(mess,20){}
    
    /**
     * Returns homing velocity.
     * @return velocity in Thorlabs specified units.
     */
    int32_t GetHomingVelocity(){ return le32toh(*((int32_t*) &bytes[12])); }
};

/**
 * Sets limit switch parameters for specified motor.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetLimitSwitchParams:public LongMessage{
public:
    SetLimitSwitchParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_LIMSWITCHPARAMS, 16, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
    
    /**
     * Sets operation of the clockwise hardware limit switch when contact is made. 
     * @param limit - code of operation\n
     * 1 - ignore switch or switch not present\n
     * 2 - switch makes on contact\n  
     * 3 - switch breaks on contact\n    
     * 4 - switch makes on contact, only used for homes\n
     * 5 - switch breaks on contact, only used for homes\n
     * 6 - For PMD based brushless servo controllers only, uses index mark for homing\n
     * 128(0x80) - bitwise OR \n   
     * @return INVALID_PARAM for invalid values, 0 on success
     */
    int SetClockwiseHardLimit(uint16_t limit){
        if ( limit != 0x80 && limit > 0x06 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(limit);
        return 0;
    }
    
    /**
     * Sets operation of the counterclockwise hardware limit switch when contact is made. 
     * @param limit - code of operation\n
     * 1 - ignore switch or switch not present\n
     * 2 - switch makes on contact\n  
     * 3 - switch breaks on contact\n    
     * 4 - switch makes on contact, only used for homes\n
     * 5 - switch breaks on contact, only used for homes\n
     * 6 - For PMD based brushless servo controllers only, uses index mark for homing\n
     * 128(0x80) - bitwise OR \n   
     * @return INVALID_PARAM for invalid values, 0 on success
     */
    int SetCounterlockwiseHardLimit(uint16_t limit){ 
        if ( limit != 0x80 && limit > 0x06 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[10]) = htole16(limit);
        return 0;
    }
    
    /**
     * Sets clockwise software limit.
     * @param limit - clockwise software limit in position steps
     * @return IGNORED_PARAM if parameter isn't applicable to controller device, 0 on succes
     */
    int SetClockwiseSoftLimit(int32_t limit){
        if (opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        *((int32_t *) &bytes[12]) = htole32(limit); 
        return 0;
    }
    
    /**
     * Sets counterclockwise software limit.
     * @param limit - clockwise software limit in position steps
     * @return IGNORED_PARAM if parameter isn't applicable to controller device, 0 on succes
     */
    int SetCounterlockwiseSoftLimit(int32_t limit){ 
        if (opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        *((int32_t *) &bytes[16]) = htole32(limit); 
        return 0;
    }  
    
    /**
     * Software limit switch mode.
     * @param mode\n
     * 1 - ignore limit\n
     * 2 - stop immediate at limit\n
     * 3 - profiled stop at limit\n
     * 128(0x80) - bitwise OR
     * @return INVALID_PARAM on invalid value, 0 on succes
     */
    int SetLimitMode(uint16_t mode){ 
        if ( opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        if ( mode != 0x80 && mode > 0x03 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[20]) = htole16(mode); 
        return 0;
    }
};

/**
 * Requests limit switch parameters for specified motor.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqLimitSwitchParams:public HeaderMessage{
public:
    ReqLimitSwitchParams(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_LIMSWITCHPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains limit switch parameters for specified motor.
 */
class GetLimitSwitchParams:public LongMessage{
public:
    GetLimitSwitchParams():LongMessage(22){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetLimitSwitchParams(uint8_t *mess):LongMessage(mess, 22){}
    
    /**
     * Return clockwise hardware limit switch operation.
     * @return \n
     * 1 - ignore switch or switch not present\n
     * 2 - switch makes on contact\n  
     * 3 - switch breaks on contact\n    
     * 4 - switch makes on contact, only used for homes\n
     * 5 - switch breaks on contact, only used for homes\n
     * 6 - For PMD based brushless servo controllers only, uses index mark for homing\n
     * 128(0x80) - bitwise OR \n   
     */
    uint16_t GetClockwiseHardLimit(){ return le16toh(*((uint16_t*) &bytes[8])); }
    
    /**
     * Return counterclockwise hardware limit switch operation.
     * @return \n
     * 1 - ignore switch or switch not present\n
     * 2 - switch makes on contact\n  
     * 3 - switch breaks on contact\n    
     * 4 - switch makes on contact, only used for homes\n
     * 5 - switch breaks on contact, only used for homes\n
     * 6 - For PMD based brushless servo controllers only, uses index mark for homing\n
     * 128(0x80) - bitwise OR \n   
     */
    uint16_t GetCounterlockwiseHardLimit(){ return le16toh(*((uint16_t*) &bytes[10])); }
    
    /**
     * Returns clockwise software limit.
     * @return limit in position steps.
     */
    int32_t SetClockwiseSoftLimit(){ return le32toh(*((int32_t*) &bytes[12])); }
    
    /**
     * Returns counterclockwise software limit.
     * @return limit in position steps.
     */
    int32_t SetCounterlockwiseSoftLimit(){ return le32toh(*((int32_t*) &bytes[16])); }
    
    /**
     * Returns mode of software limit switch.
     * @return  \n
     * 1 - ignore limit\n
     * 2 - stop immediate at limit\n
     * 3 - profiled stop at limit\n
     * 128(0x80) - bitwise OR
     */
    uint16_t GetLimitMode(){ return le16toh(*((uint16_t*) &bytes[20])); }
};

/**
 * Starts homing move.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class MoveHome:public HeaderMessage{
public:
    MoveHome(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(MOVE_HOME, chanId, 0, dest, source){}
};

/**
 * Controller response on completing homing move.
 */
class MovedHome:public HeaderMessage{
public:
    MovedHome():HeaderMessage(){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    MovedHome(uint8_t *mess):HeaderMessage(mess){}
};

/**
 * Starts relative move with parameters set in controller device.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class MoveRelative1:public HeaderMessage{
public:
    MoveRelative1(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(MOVE_RELATIVE, chanId, 0, dest, source){}
};

/**
 * Starts relative move with variable relative distance.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class MoveRelative2:public LongMessage{
public:
    MoveRelative2(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(MOVE_RELATIVE, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    /**
     * Sets relative distance.
     * @param dist - distance in Thorlabs specified units
     */
    void SetRelativeDistance(int32_t dist){ *((int32_t *) &bytes[8]) = htole32(dist); }
};

/**
 * Device response upon completing move.
 */
class MoveCompleted:public LongMessage{
public:
    MoveCompleted(uint8_t *mess):LongMessage(mess, 20){};
    
    MoveCompleted():LongMessage(20){}
};

/**
 * Starts absolute move with parameters set in controller device.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class MoveAbsolute1:public HeaderMessage{
public:
    MoveAbsolute1(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(MOVE_ABSOLUTE, chanId, 0 , dest, source){}
};

/**
 * Starts absolute move with variable absolute position.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class MoveAbsolute2:public LongMessage{
public:
    MoveAbsolute2(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(MOVE_ABSOLUTE, 6, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    };
    
    /**
     * Sets absolute position to move to
     * @param pos - absolute position to move to
     * @return INVALID PARAM on value exceeding maximal position, 0 on success
     */
    int SetAbsoluteDistance(int32_t pos){
        if (pos < 0 || pos > opened_device.motor[GetMotorID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[8]) = htole32(pos); 
        return 0;
    }
};

/**
 * Starts jog move with parameters set in device.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class JogMove:public HeaderMessage{
public:
    JogMove(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(MOVE_JOG, chanId, 1, dest, source){}
    
    /**
     * Sets direction
     * @param direction - 1 or 2
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetDirection(uint8_t direction){
        if (direction != 0x01 && direction != 0x02) return INVALID_PARAM;
        SetSecondParam(direction);
        return 0;
    }
};

/**
 * Starts move with velocity set in device.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class MovewVelocity:public HeaderMessage{
public:
    MovewVelocity(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(MOVE_VELOCITY,chanId, 1, dest, source){}
    
    /**
     * Sets direction of move
     * @param direction - 1 or 2
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetDirection(uint8_t direction){
        if (direction != 0x01 && direction != 0x02) return INVALID_PARAM;
        SetSecondParam(direction);
        return 0;
    }
};

/**
 * Stops movement.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class StopMove:public HeaderMessage{
public:
    StopMove(uint8_t dest, uint8_t source, uint8_t chanId ):HeaderMessage(MOVE_STOP, chanId, 2, dest, source){}
    
    /**
     * Sets stop mode.
     * @param mode, 1 for immediate stop, 2 for profiled stop
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetStopMode(uint8_t mode){
        if (mode != 0x01 && mode != 0x02) return INVALID_PARAM;
        SetSecondParam(mode);
        return 0;
    }
};

/**
 * Device response upon move has stopped.
 */
class MoveStopped: public LongMessage{
public:
    MoveStopped():LongMessage(20){}
    
    MoveStopped(uint8_t *mess):LongMessage(mess,20){}
}; 

/**
 * Sets acceleration and deceleration profile
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetBowIndex:public LongMessage{
public:
    SetBowIndex(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_BOWINDEX, 4, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    /**
     * Sets profile of acceleration/deceleration
     * @param 0 for trapezoidal, 1-18 for s-curve profile
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetBowindex(uint16_t index){
        if ( index > 18 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(index); 
        return 0;
    }
};

/**
 * Requests acceleration and deceleration profile
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqBowIndex:public HeaderMessage{
public:
    ReqBowIndex(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_BOWINDEX, chanId, 0, dest, source){}
};

/**
 * Contains acceleration and deceleration profile
 */
class GetBowIndex:public LongMessage{
public:
    GetBowIndex():LongMessage(10){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetBowIndex(uint8_t *mess):LongMessage(mess, 10){};
    
    /**
     * Return profile.
     * @return 0 means trapezoidal, 1-18 means s-curve profile
     */
    uint16_t BowIndex(){ return le16toh(*((uint16_t*) &bytes[8])); }
};

/**
 * Sets mode of front LEDs.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetLedMode:public LongMessage{
public:
    SetLedMode(uint8_t dest, uint8_t source, uint16_t chanId):LongMessage(SET_AVMODES, 4, dest, source){
        *((uint16_t *) &bytes[6]) = htole16(chanId);
    }
    
    /**
     * Sets mode for LED. Combinations are possible.
     * @param mode\m
     * 1 - flash on identification message\n
     * 2 - flash when motor reach limit switch\n
     * 8 - lit while motor is moving\n
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetMode(uint16_t mode){ 
        if ( mode != 1 && mode != 2 && mode != 3 && (mode < 8  || mode > 11)) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(mode); 
         return 0;
    }
       
};

/**
 * Requests mode of front LEDs.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqLedMode:public HeaderMessage{
public:
    ReqLedMode(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_AVMODES, chanId, 0, dest, source){}
};

/**
 * Contains mode of front LEDs.
 */
class GetLedMode:public LongMessage{
public:
    GetLedMode():LongMessage(10){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetLedMode(uint8_t *mess):LongMessage(mess, 10){}
    
    /**
     * Returns mode of LED. May contain combination of modes.
     * @return \n
     * 1 - flash on identification message\n
     * 2 - flash when motor reach limit switch\n
     * 8 - lit while motor is moving\n
     */
    uint16_t GetMode(){ return le16toh(*((uint16_t*) &bytes[8])); } 
};

/**
 * Sets controller device buttons parameters.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class SetButtonParams:public LongMessage{
public:
    SetButtonParams(uint8_t dest, uint8_t source, uint16_t chanId)
            :LongMessage(SET_BUTTONPARAMS, 16, dest, source){
                *((uint16_t *) &bytes[6]) = htole16(chanId);
            }
    
    /**
     * Sets mode of buttons. Upon pressing button motor can either perform jog move or
     * move to preset absolute position.
     * @param mode - 1 for jog, 2 for absolute move
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetMode(uint16_t mode){
        if (mode != 1 && mode != 2 ) return INVALID_PARAM;
        *((uint16_t *) &bytes[8]) = htole16(mode); 
        return 0;
    }
    
    /**
     * Sets absolute position to move to after pressing button 1.
     * @param pos - position in Thorlabs specified units.
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetPosition1(int32_t pos){
        if (pos < 0 || pos > opened_device.motor[GetMotorID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[10]) = htole32(pos);
        return 0;
    }
    
    /**
     * Sets absolute position to move to after pressing button 2.
     * @param pos - position in Thorlabs specified units.
     * @return INVALID_PARAM on invalid value, 0 on success
     */
    int SetPosition2(int32_t pos){
        if (pos < 0 || pos > opened_device.motor[GetMotorID()].max_pos ) return INVALID_PARAM;
        *((int32_t *) &bytes[14]) = htole32(pos);
        return 0;
    }
    
    int SetTimeout(uint16_t ms){ 
        if (opened_device.device_type == TDC001 ) return IGNORED_PARAM;
        *((uint16_t *) &bytes[18]) = htole16(ms); 
        return 0;
    }
};

/**
 * Requests controller device buttons parameters.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqButtonParams:public HeaderMessage{
public:
    ReqButtonParams(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_BUTTONPARAMS, chanId, 0, dest, source){}
};

/**
 * Contains controller device buttons parameters.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class GetButtonParams: public LongMessage{
public:
    GetButtonParams():LongMessage(22){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetButtonParams(uint8_t *mess):LongMessage(mess, 22){}
    
    /**
     * Return mode of controller device buttons. Upon pressing button motor can either perform jog move or
     * move to preset absolute position.
     * @return 1 for jog mode, 2 for absolute move mode
     */
    uint16_t GetMode(){ return le16toh(*((uint16_t*) &bytes[8])); }
    
    /**
     * Returns position to move to after pressing button 1.
     * @return position in Thorlabs specified units
     */
    int32_t GetPosition1(){ return le32toh(*((int32_t*) &bytes[12])); }
    
    /**
     * Returns position to move to after pressing button 2.
     * @return position in Thorlabs specified units
     */
    int32_t GetPosition2(){ return le32toh(*((int32_t*) &bytes[14])); }
    
    uint16_t GetTimeout(){ return le16toh(*((uint16_t*) &bytes[18])); }
};

/**
 * Requests status update.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqStatusUpdate:public HeaderMessage{
public: 
    ReqStatusUpdate(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_STATUSUPDATE, chanId, 0, dest, source){}
};

/**
 * Contains status update.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class GetStatusUpdate:public LongMessage{
public:
    GetStatusUpdate():LongMessage(20){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetStatusUpdate(uint8_t *mess):LongMessage(mess,20){}
    
    /**
     * Returns position.
     * @return position in Thorlabs specified units
     */
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
    /**
     * Returns encoder counter value.
     * @return encoder count
     */
    int32_t GetEncCount(){ return le32toh(*((int32_t*) &bytes[12])); }
    /**
     * Returns word containing status bits.
     * @return status word
     */
    uint32_t GetStatusBits(){ return le32toh(*((uint32_t*) &bytes[16])); }
};

/**
 * Requests status update.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqMotChanStatusUpdate:public HeaderMessage{
public:
    ReqMotChanStatusUpdate(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_DCSTATUSUPDATE, chanId, 0, dest, source){}
};

/**
 * Contains status update.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class GetMotChanStatusUpdate:public LongMessage{
public:
    GetMotChanStatusUpdate():LongMessage(20){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetMotChanStatusUpdate(uint8_t *mess):LongMessage(mess,20){}
    
    /**
     * Returns actual position.
     * @return position in Thorlabs specified units
     */
    int32_t GetPosition(){ return le32toh(*((int32_t*) &bytes[8])); }
    /**
     * Returns actual velocity.
     * @return velocity in Thorlabs specified units
     */
    uint16_t GetVelocity(){ return le16toh(*((uint16_t*) &bytes[12])); }
    /**
     * Returns word containing status bits.
     * @return status word
     */
    uint32_t GetStatusBits(){ return le32toh(*((uint32_t*) &bytes[16])); }
};

/**
 * Notifies that server is connected.
 * @param dest - destination address
 * @param source - source destination
 */
class ServerAlive:public HeaderMessage{
public:
    ServerAlive(uint8_t dest, uint8_t source):HeaderMessage(ACK_DCSTATUSUPDATE, 0, 0, dest, source){}
};

/**
 * Requests message containing status.
 * @param dest - destination address
 * @param source - source destination
 * @param chanID - channel ID
 */
class ReqStatusBits:public HeaderMessage{
public:
    ReqStatusBits(uint8_t dest, uint8_t source,  uint8_t chanId):HeaderMessage(REQ_STATUSBITS, chanId, 0, dest, source){}
};

/**
 * Contains bits describing status.
 */
class GetStatusBits:public LongMessage{
public:
    GetStatusBits():LongMessage(12){}
    
    /**
     * Construct message and copies buffer to message.
     * @param mess - pointer to buffer to copy
     */
    GetStatusBits(uint8_t *mess):LongMessage(mess,12){}
    
    /**
     * Returns word containing status bits.
     * @return 32 bit long status word
     */
    uint32_t GetStatBits(){ return le32toh(*((uint32_t*) &bytes[8])); }
};

/**
 * Disables sending end of move messages from controller device.
 * @param dest - destination address
 * @param source - source destination
 */
class DisableEndMoveMessages:public HeaderMessage{
public:
    DisableEndMoveMessages(uint8_t dest, uint8_t source):HeaderMessage(SUSPEND_ENDOFMOVEMSGS, 0, 0, dest, source){}
};

/**
 * Enables sending end of move messages from controller device.
 * @param dest - destination address
 * @param source - source destination
 */
class EnableEndMoveMessages:public HeaderMessage{
public:
    EnableEndMoveMessages(uint8_t dest, uint8_t source):HeaderMessage(RESUME_ENDOFMOVEMSGS, 0, 0, dest, source){}
};

class SetTrigger:public HeaderMessage{
public:
    SetTrigger(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(SET_TRIGGER, chanId, 0, dest, source){}
    
    int SetMode(uint8_t mode){
        if (opened_device.device_type != BSC201 && opened_device.device_type != BSC202 && opened_device.device_type != BSC203 && 
            opened_device.device_type != TBD001 && opened_device.device_type != BBD201 && opened_device.device_type != BBD202 && 
            opened_device.device_type != BBD203) 
            return IGNORED_PARAM;
        SetSecondParam(mode);        
        return 0;        
    }
    
};

class ReqTrigger:public HeaderMessage{
public:
    ReqTrigger(uint8_t dest, uint8_t source, uint8_t chanId):HeaderMessage(REQ_TRIGGER, chanId, 0, dest, source){}
};

class GetTrigger:public HeaderMessage{
public:
    GetTrigger():HeaderMessage(){}
    
    GetTrigger(uint8_t *mess):HeaderMessage(mess){}
    
    uint8_t GetMode(){return GetSecondParam() ;}
};

 #endif 