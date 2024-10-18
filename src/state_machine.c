#include "state_machine.h"

state s;

void state_machine(unsigned char byte, LinkLayerRole role) {
    if (role == LlRx) {
        switch (s)
        {
            case START:
                printf("START\n");
                if (byte == FLAG) {
                    s = FLAG_RCV;
                }
                break;
            
            case FLAG_RCV:
                printf("FLAG_RCV\n");
                if (byte == A_SENDER) {
                    s = A_RCV;
                }
                else {
                    s = START;
                }
                break;
            
            case A_RCV:
                printf("A_RCV\n");
                if (byte == C_SET) {
                    s = C_RCV;
                }
                else {
                    s = START;
                }
                break;
            
            case C_RCV:
                printf("C_RCV\n");
                if (byte == (A_SENDER ^ C_SET)) {
                    s = BCC_OK;
                }
                else {
                    s = START;
                }
                break;
            
            case BCC_OK:
                printf("BCC_OK\n");
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
