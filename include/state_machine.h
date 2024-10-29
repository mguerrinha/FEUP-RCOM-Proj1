#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "link_layer.h"

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    BCC1_OK,
    DATA_PACKET,
    ESC_RCV,
    DATA_RCV,
    BCC2_OK,
    STOP_RCV
} state;

int state_machine_connection(unsigned char byte, LinkLayerRole role);

int state_machine_end_connection(unsigned char byte, volatile int receiver);

int state_machine_transmitter(unsigned char byte);

int state_machine_receiver(unsigned char byte, unsigned char *packet);


#endif