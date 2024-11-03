#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "link_layer.h"

#define FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_RR0 0xAA
#define C_RR1 0xAB
#define C_REJ0 0x54
#define C_REJ1 0x55
#define C_DISC 0x0B
#define C_0 0x00
#define C_1 0x80
#define ESC 0x7D
#define STUFF 0x20

// States to confirm frames and packets
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

// State machine to confirm the bytes received from the connection fase
int state_machine_connection(unsigned char byte, LinkLayerRole role);

// State machine to confirm the bytes received from the end connection fase
int state_machine_end_connection(unsigned char byte, volatile int receiver);

// State machine to confirm the bytes received by the Transmitter
int state_machine_transmitter(unsigned char byte);

// State machine to confirm the bytes received by the Receiver
int state_machine_receiver(unsigned char byte, unsigned char *packet);


#endif