#include "state_machine.h"

state s;
extern unsigned int frameNumber;

void state_machine_connection(unsigned char byte, LinkLayerRole role) {
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
                }
                break;
            
            case A_RCV:
                if (byte == C_SET) {
                    s = C_RCV;
                }
                else {
                    s = START;
                }
                break;
            
            case C_RCV:
                if (byte == (A_SENDER ^ C_SET)) {
                    s = BCC_OK;
                }
                else {
                    s = START;
                }
                break;
            
            case BCC_OK:
                if (byte == FLAG) {
                    s = STOP_RCV;
                }
                else {
                    s = START;
                }
                break;
            default:
                s = START;
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
                }
                break;
            
            case A_RCV:
                if (byte == C_UA) {
                    s = C_RCV;
                }
                else {
                    s = START;
                }
                break;
            
            case C_RCV:
                if (byte == (A_RECEIVER ^ C_UA)) {
                    s = BCC_OK;
                }
                else {
                    s = START;
                }
                break;
            
            case BCC_OK:
                if (byte == FLAG) {
                    s = STOP_RCV;
                }
                else {
                    s = START;
                }
                break;
            default:
                s = START;
                break;
        }
    }
}

void state_machine_transmitter(unsigned char byte) {
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
        }
        break;
    
    case A_RCV:
        if (byte == C_RR0 || byte == C_RR1) {
            s = C_RCV;
        }
        else {
            s = START;
        }
        break;
    
    case C_RCV:
        if (frameNumber % 2 == 0) {
            if (byte == C_RR1 ^ A_RECEIVER) {
                s = BCC_OK;
            }
            else {
                s = START;
            }
        }
        else if (frameNumber % 2 == 1) {
            if (byte == C_RR0 ^ A_RECEIVER) {
                s = BCC_OK;
            }
            else {
                s = START;
            }
        }
        break;
    
    case BCC_OK:
        if (byte == FLAG) {
            s = STOP_RCV;
        }
        else {
            s = START;
        }
        break;
    default:
        s = START;
        break;
    }
}
