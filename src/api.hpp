/*
 * Api for thorlabs messages.
 */
#include "../ftdi_lib/ftd2xx.h"

typedef struct {
    uint8_t bytes[6];
} message_header_t;

typedef struct {
    uint8_t *bytes;
    unsigned int size;
} message_body_t;