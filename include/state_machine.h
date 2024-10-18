#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#define FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07

#include "link_layer.h"

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP_RCV
} state;

void state_machine(unsigned char byte, LinkLayerRole role);

#endif