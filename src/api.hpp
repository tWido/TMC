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


class MessageHeader: public Message{
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

class LongMessage: public Message{
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

int SendMessage(Message message, FT_HANDLE &handle){
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

// Generic messages -------------------------------------------------------------

/** Flash front panel LED. */
class IdentifyMs: public MessageHeader{
public:
    IdentifyMs(uint8_t dest = 0x50, uint8_t source = 0x01 ):MessageHeader(IDENTIFY, 0, 0, dest, source){}
};


/** Enable or disable drive channel. */
class SetChannelState:MessageHeader{
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
class ReqChannelState:MessageHeader{
    ReqChannelState(uint8_t chanID = 0x01, uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(REQ_CHANENABLESTATE, chanID, 0, dest, source){};
   
    /** 
     * @brief Set channel id which info is required. 
     * @param chanID - starting at 0x01
     */
    void SetChannelIdent(uint8_t chanID){ SetFirstParam(chanID); }
};

/** Info sent from device. */
class ChannelStateInfo:MessageHeader{
    ChannelStateInfo(uint8_t *mess):MessageHeader(mess){}
    
    /**
     * @brief Saves info in given variables.
     * @param chanID
     * @param state - 0x01 = enabled, 0x02 = disabled
     */
    void Getinfo(uint8_t *chanID, uint8_t *state){ GetParams(chanID, state); }
};

class HwDisconnect:MessageHeader{
    HwDisconnect(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader( HW_DISCONNECT, 0, 0, dest, source){}
};

/** Sent from device to notify of unexpected event. */
class HwResponse:MessageHeader{
    HwResponse(uint8_t *mess):MessageHeader(mess){}
};

/** Sent from device to specify error. */
class HwResponseInfo:LongMessage{
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

class StartUpdateMessages:MessageHeader{
    StartUpdateMessages(uint8_t rate = 0, uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(HW_START_UPDATEMSGS, rate, 0, dest, source){};
    
    void SetUpdaterate(uint8_t rate){ SetFirstParam(rate); }
};

class StopUpdateMessages:MessageHeader{
    StopUpdateMessages(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(HW_STOP_UPDATEMSGS, 0, 0, dest, source){}
};

class ReqHwInfo:MessageHeader{
    ReqHwInfo(uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(HW_REQ_INFO, 0, 0, dest, source){}
};

class HwInfo:LongMessage{
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
  
    
    class ReqRackBayUsed:MessageHeader{
        ReqRackBayUsed(uint8_t bayID, uint8_t dest = 0x50, uint8_t source = 0x01):MessageHeader(RACK_REQ_BAYUSED, bayID, 0 , dest, source){}
        
        void SetBayIdent(uint8_t bayID){ SetFirstParam(bayID); }
    };
    
    class GetRackBayUsed:MessageHeader{
        GetRackBayUsed(uint8_t *mess):MessageHeader(mess){}
        
        uint8_t GetBayID(){ return GetFirstParam(); }
        
        /**
         * @return Baystates: 0x01 - bay occupied, 0x02 - bay empty 
         */
        uint8_t GetBayState(){ return GetSecondParam(); }
    };
    
    class ReqHubBayUsed:MessageHeader{
        ReqHubBayUsed(uint8_t dest = 0x50, uint8_t source = 0x01 ):MessageHeader(HUB_REQ_BAYUSED, 0, 0, dest, source){}
    };
    
    class GetBayUsed:MessageHeader{
        GetBayUsed(uint8_t *mess):MessageHeader(mess){}
        
        uint8_t GetBayID(){ return GetFirstParam(); }
    };
    
    
    //Motor control messages ---------------------------------------------------
    
};