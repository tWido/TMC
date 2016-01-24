/*
 * Api for thorlabs messages.
 */
#include "../ftdi_lib/ftd2xx.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define HEADER_SIZE 6

class Message{
public:
    Message(char* buffer, unsigned int buffer_size){
        bytes = (char *) malloc(buffer_size);
        bytes = strncpy(bytes, buffer, buffer_size);
        lenght = buffer_size;
    }
    
    Message(size_t in_size){
        bytes = (char *) malloc(in_size);
    }
    
    ~Message(){
        free(bytes);
    }
 
    char* data(){ return bytes; }
    
    unsigned int msize(){ return lenght; }
    
protected:
    unsigned int lenght;
    char *bytes;
};


class MessageHeader: Message{
public:
    MessageHeader(char *header_bytes): Message(header_bytes, HEADER_SIZE){
    //not implemented
    };
    
    MessageHeader(uint16_t type, uint8_t dest, uint8_t source):Message(HEADER_SIZE){
    //not implemented
    };
    
    MessageHeader(uint16_t type, uint8_t param1, uint8_t param2 ,uint8_t dest, uint8_t source):Message(HEADER_SIZE){
    //not implemented
    };
    
    bool HeaderOnly(){ return header_only; };
    void Getparams(uint8_t *p1, uint8_t *p2){
        *p1 =(uint8_t) bytes[2];
        *p2 =(uint8_t) bytes[3];
    };
    
    void SetParams(uint8_t p1, uint8_t p2){
        bytes[2] = p1;
        bytes[3] = p2;
    };
    
private: 
    bool header_only;
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

