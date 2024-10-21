#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "link_layer.h"

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP_RCV
} state;

void state_machine_connection(unsigned char byte, LinkLayerRole role);

void state_machine_transmitter(unsigned char byte);

#endif