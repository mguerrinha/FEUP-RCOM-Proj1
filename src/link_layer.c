// Link layer protocol implementation

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "alarm.h"
#include "link_layer.h"
#include "serial_port.h"
#include "state_machine.h"

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 5

volatile int STOP = FALSE;
extern volatile int alarmEnabled;
extern int alarmCount;
extern state s;

int nRetransmissions;
extern int nRetransmissionsTotal;
int timeout;
volatile int receiver = -1;

unsigned int frameNumber;
int i = 0;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    nRetransmissions = connectionParameters.nRetransmissions;
    nRetransmissionsTotal = 0;
    timeout = connectionParameters.timeout;

    int fd = openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate);
    if (fd < 0) {
        return -1;
    }


    printf("New termios structure set\n");

    STOP = FALSE;

    if (connectionParameters.role == LlTx) {
        receiver = FALSE;
        unsigned char buf_set[BUF_SIZE] = {FLAG, A_SENDER, C_SET, A_SENDER^C_SET, FLAG};
        unsigned char buf_ua[BUF_SIZE];
        setSignal();

        while (STOP == FALSE && alarmCount <= nRetransmissions)
        {
            if (alarmEnabled == FALSE) {
                int bytes = writeBytesSerialPort(buf_set, BUF_SIZE);
                printf("%d bytes written\n", bytes);
                setAlarm(timeout);
            }

            s = START;

            while (s != STOP_RCV && alarmEnabled == TRUE)
            {
                int bytes = readByteSerialPort(buf_ua);
                if (bytes > 0)
                    if (state_machine_connection(buf_ua[0], connectionParameters.role)) {
                        break;
                    }
            }

            if (s == STOP_RCV) {
                STOP = TRUE;
            }
        }            
    }
    else if (connectionParameters.role == LlRx) {
        receiver = TRUE;
        unsigned char buf_set[BUF_SIZE];

        s = START;

        while (s != STOP_RCV)
        {
            int bytes = readByteSerialPort(buf_set);
            if (bytes > 0) {
                if (state_machine_connection(buf_set[0], connectionParameters.role)) {
                    continue;
                }
            }
        }

        unsigned char buf_ua[BUF_SIZE] = {FLAG, A_RECEIVER, C_UA, A_RECEIVER^C_UA, FLAG};
        int bytes = writeBytesSerialPort(buf_ua, BUF_SIZE);
        printf("%d bytes written\n", bytes);
    }
    if (alarmCount > nRetransmissions) {
        return -1;
    }
    frameNumber = 0;
    return fd;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    int frameSize = 6 + bufSize;
    unsigned char *frame = (unsigned char*)malloc(frameSize);
    if (frame == NULL) {
        perror("Memory allocation failed\n");
        return -1;
    }

    // Config the transmitter frame

    frame[0] = FLAG;
    frame[1] = A_SENDER;
    if (frameNumber % 2 == 0) {
        frame[2] = C_0;
    }
    else if (frameNumber % 2 == 1) {
        frame[2] = C_1;
    }
    frame[3] = A_SENDER ^ frame[2];
    memcpy(frame+4, buf, bufSize);
    
    // Construction of the BCC2 with original data
    unsigned char BCC2 = buf[0];
    for (unsigned int i = 1; i < bufSize; i++) {
        BCC2 ^= buf[i];
    }

    // Aplying stuffing in data
    int j = 4;
    for (unsigned int i = 0; i < bufSize; i++) {
        if (buf[i] == FLAG) {
            frame = realloc(frame, frameSize+1);
            frame[j++] = ESC;
            frame[j++] = FLAG ^ STUFF;
            frameSize++;
        }
        else if (buf[i] == ESC) {
            frame = realloc(frame, frameSize+1);
            frame[j++] = ESC;
            frame[j++] = ESC ^ STUFF;
            frameSize++;
        }
        else {
            frame[j++] = buf[i];
        }
    }

    if (BCC2 == FLAG) {
        frame = realloc(frame, frameSize+1);
        frame[j++] = ESC;
        frame[j++] = FLAG ^ STUFF;
        frameSize++;
    }
    else if (BCC2 == ESC) {
        frame = realloc(frame, frameSize+1);
        frame[j++] = ESC;
        frame[j++] = ESC ^ STUFF;
        frameSize++;
    }
    else {
        frame[j++] = BCC2;
    }

    frame[j] = FLAG;

    alarmEnabled = FALSE;
    alarmCount = 0;

    unsigned char buf_rc[BUF_SIZE];

    STOP = FALSE;

    int bytesWritten;


    while (STOP == FALSE && alarmCount <= nRetransmissions)
    {
        if (alarmEnabled == FALSE) {
            bytesWritten = writeBytesSerialPort(frame, frameSize);
            setAlarm(timeout);
        }

        s = START;

        while (alarmEnabled == TRUE && s != STOP_RCV)
        {
            int bytes = readByteSerialPort(buf_rc);
            if (bytes > 0) {
                alarmCount = 0;
                if (state_machine_transmitter(buf_rc[0])) {
                    break;
                }
            }
        }

        if (s == STOP_RCV) {
            STOP = TRUE;
        }
    }
    if (alarmCount >= nRetransmissions) {
        return -1;
    }
    frameNumber++;
    return bytesWritten;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    s = START;
    unsigned char buf_rc[BUF_SIZE];
    unsigned char buf[BUF_SIZE];
    buf[0] = FLAG;
    buf[1] = A_RECEIVER;
    i = 0;

    while (s != STOP_RCV)
    {
        int bytes = readByteSerialPort(buf_rc);
        
        if (bytes > 0) {
            if (state_machine_receiver(buf_rc[0], packet)) {
                buf[2] = (frameNumber % 2 == 0) ? C_REJ0 : C_REJ1;
                break;
            }
        }
        if (s == STOP_RCV) {
            buf[2] = (frameNumber % 2 == 0) ? C_RR1 : C_RR0;
            frameNumber++;
        }
    }

    buf[3] = (A_RECEIVER ^ buf[2]);
    buf[4] = FLAG;
    writeBytesSerialPort(buf, BUF_SIZE);
    
    if (s != STOP_RCV) {
        return -1;
    }
    return i-1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO
    STOP = FALSE;
    alarmCount = 0;
    alarmEnabled = FALSE;

    if (!receiver) {
        unsigned char buf_disc[BUF_SIZE] = {FLAG, A_SENDER, C_DISC, A_SENDER ^ C_DISC, FLAG};
        unsigned char buf[BUF_SIZE];
        printf("TRANSMITTER\n");

        while (STOP == FALSE && alarmCount <= nRetransmissions)
        {
            if (alarmEnabled == FALSE) {
                writeBytesSerialPort(buf_disc, BUF_SIZE);
                setAlarm(timeout);
            }

            s = START;

            while (s != STOP_RCV && alarmEnabled == TRUE)
            {
                int bytes = readByteSerialPort(buf);
                if (bytes > 0) {
                    if (state_machine_end_connection(buf[0], receiver)) {
                        break;
                    }
                }
            }
            if (s == STOP_RCV) {
                STOP = TRUE;
            }
        }
        unsigned char buf_ua[BUF_SIZE] = {FLAG, A_SENDER, C_UA, A_SENDER ^ C_UA, FLAG};
        writeBytesSerialPort(buf_ua, BUF_SIZE);
        setAlarm(0);

        if (alarmCount > nRetransmissions) {
            return 1;
        }
        
        setAlarm(0);

        if (showStatistics) {
            printf("STATISICS\n");
            printf("----------------------------\n");
            printf("NUMBER FRAMES SENT --> %d\n", frameNumber);
            printf("----------------------------\n");
            printf("RETRANSMISSIONS    --> %d\n", nRetransmissionsTotal);
            printf("----------------------------\n");
        }
    }
    else if (receiver) {
        unsigned char buf_disc[BUF_SIZE];
        printf("RECEIVER\n");

        s = START;

        while (s != STOP_RCV)
        {
            int bytes = readByteSerialPort(buf_disc);
            if (bytes > 0) {
                if (state_machine_end_connection(buf_disc[0], receiver)) {
                    continue;
                }
            }
        }

        STOP = FALSE;
        alarmCount = 0;

        unsigned char buf[BUF_SIZE] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER ^ C_DISC, FLAG};


        while (STOP == FALSE && alarmCount <= nRetransmissions)
        {
            if (alarmEnabled == FALSE) {
                writeBytesSerialPort(buf, BUF_SIZE);
                setAlarm(timeout);
            }

            s = START;

            while (s != STOP_RCV && alarmEnabled == TRUE)
            {
                int bytes = readByteSerialPort(buf_disc);
                if (bytes > 0) {
                    if (state_machine_end_connection(buf_disc[0], receiver)) {
                        continue;
                    }
                }
            }
            if (s == STOP_RCV) {
                STOP = TRUE;
            }
        }
        if (alarmCount > nRetransmissions) {
            return 1;
        }

        setAlarm(0);
        
        if (showStatistics) {
            printf("STATISTICS\n");
            printf("----------------------------\n");
            printf("NUMBER FRAMES RECEIVED --> %d\n", frameNumber);
            printf("----------------------------\n");
        }
    }

    int clstat = closeSerialPort();
    return clstat;
}
