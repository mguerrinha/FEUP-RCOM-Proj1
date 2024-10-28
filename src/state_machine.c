#include "state_machine.h"
#include <stdio.h>


state s;
extern unsigned int frameNumber;
extern int i;
unsigned char bcc2;
unsigned char bcc2_rcv;

int state_machine_connection(unsigned char byte, LinkLayerRole role) {
    if (role == LlRx) {
        switch (s)
        {
            case START:
                if (byte == FLAG) {
                    s = FLAG_RCV;
                }
                break;
            
            case FLAG_RCV:
                if (byte == A_SENDER) {
                    s = A_RCV;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            
            case A_RCV:
                if (byte == C_SET) {
                    s = C_RCV;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            
            case C_RCV:
                if (byte == (A_SENDER ^ C_SET)) {
                    s = BCC_OK;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            
            case BCC_OK:
                if (byte == FLAG) {
                    s = STOP_RCV;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            default:
                s = START;
                return 1;
                break;
        }
    }
    else if (role == LlTx) {
        switch (s)
        {
        case START:
                if (byte == FLAG) {
                    s = FLAG_RCV;
                }
                break;
            
            case FLAG_RCV:
                if (byte == A_RECEIVER) {
                    s = A_RCV;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            
            case A_RCV:
                if (byte == C_UA) {
                    s = C_RCV;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            
            case C_RCV:
                if (byte == (A_RECEIVER ^ C_UA)) {
                    s = BCC_OK;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            
            case BCC_OK:
                if (byte == FLAG) {
                    s = STOP_RCV;
                }
                else {
                    s = START;
                    return 1;
                }
                break;
            default:
                break;
        }
    }
    return 0;
}

int state_machine_transmitter(unsigned char byte) {
    unsigned char control = (frameNumber % 2 == 0) ? C_RR1 : C_RR0;
    switch (s)
    {
    case START:
        if (byte == FLAG) {
            s = FLAG_RCV;
        }
        break;
    
    case FLAG_RCV:
        if (byte == A_RECEIVER) {
            s = A_RCV;
        }
        else {
            s = START;
            return 1;
        }
        break;
    
    case A_RCV:
        if (byte == control) {
            s = C_RCV;
        }
        else {
            s = START;
            return 1;
        }
        break;
    
    case C_RCV:
        if (byte == (A_RECEIVER ^ control)) {
            s = BCC_OK;
        }
        else {
            s = START;
            return 1;
        }
        break;
    
    case BCC_OK:
        if (byte == FLAG) {
            s = STOP_RCV;
        }
        else {
            s = START;
            return 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

int state_machine_receiver(unsigned char byte, unsigned char *packet) {
    unsigned char control = (frameNumber % 2 == 0) ? C_0 : C_1;
    
    switch (s)
    {
    case START:
        if (byte == FLAG) {
            s = FLAG_RCV;
        }
        break;
    
    case FLAG_RCV:
        if (byte == A_SENDER) {
            s = A_RCV;
        }
        else {
            s = START;
            printf("ADRESS FAILED\n");
            return 1;
        }
        break;
    
    case A_RCV:
        if (byte == control) {
            s = C_RCV;
        }
        else {
            s = START;
            printf("CONTROL FAILED\n");
            return 1;
        }
        break;
    
    case C_RCV:
        if (byte == (A_SENDER ^ control)) {
            s = DATA_PACKET;
        }
        else {
            s = START;
            printf("BCC1 FAILED\n");
            return 1;
        }
        break;

    case DATA_PACKET:
        if (byte == ESC) {
            s = ESC_RCV;
        }
        else if (byte == FLAG) {
            bcc2_rcv = packet[i-1];
            bcc2 = packet[0];
            for (int j = 1; j < i-1; j++) {
                bcc2 ^= packet[j];
            }
            if (bcc2 == bcc2_rcv) {
                s = STOP_RCV;
                return 0;
            }
            else {
                s = START;
                printf("BCC2 FAILED");
                return 1;
            }
        }
        else {
            packet[i] = byte;
            i++;  
        }
        break;
    
    case ESC_RCV:
        s = DATA_PACKET;
        if (byte == (FLAG ^ STUFF)) {
            packet[i] = FLAG;
            i++;
        }
        else if (byte == (ESC ^ STUFF)) {
            packet[i] = ESC;
            i++;
        }
        else {
            s = START;
            return 1;
        }
        break;

    default:
        break;
    }
    return 0;
}