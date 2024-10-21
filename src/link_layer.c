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
extern int alarmEnabled;
extern int alarmCount;
extern state s;

int nRetransmissions;
int timeout;

unsigned int frameNumber = 0;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    nRetransmissions = connectionParameters.nRetransmissions;
    timeout = connectionParameters.timeout;

    int fd = openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate);
    if (fd < 0) {
        return -1;
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    // TODO
    STOP = FALSE;
    int n_bytes_received = 0;

    if (connectionParameters.role == LlTx) {
        unsigned char buf_set[BUF_SIZE] = {FLAG, A_SENDER, C_SET, A_SENDER^C_SET, FLAG};
        unsigned char buf_ua[BUF_SIZE];
        setSignal();

        while (STOP == FALSE && alarmCount < nRetransmissions)
        {
            if (alarmEnabled == FALSE) {
                int bytes = writeBytesSerialPort(buf_set, BUF_SIZE);
                printf("%d bytes written\n", bytes);
                sleep(1);
                setAlarm(timeout);
            }

            s = START;
            n_bytes_received = 0;

            while (s != STOP_RCV && n_bytes_received <= 5)
            {
                int bytes = readByteSerialPort(buf_ua);
                n_bytes_received++;
                printf("Bytes received: %d\n", bytes);
                if (bytes > 0)
                    state_machine_connection(buf_ua[0], connectionParameters.role);
            }

            if (s == STOP_RCV) {
                STOP = TRUE;
            }
        }            
    }
    else if (connectionParameters.role == LlRx) {
        unsigned char buf_set[BUF_SIZE];

        s = START;

        while (s != STOP_RCV)
        {
            int bytes = readByteSerialPort(buf_set);
            printf("Bytes received: %d\n", bytes);
            if (bytes > 0) {
                state_machine_connection(buf_set[0], connectionParameters.role);
            }
        }

        unsigned char buf_ua[BUF_SIZE] = {FLAG, A_RECEIVER, C_UA, A_RECEIVER^C_UA, FLAG};
        int bytes = writeBytesSerialPort(buf_ua, BUF_SIZE);
        printf("%d bytes written\n", bytes);
        sleep(1);
    }
    if (alarmCount >= nRetransmissions) {
        return 0;
    }
    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
        // TODO
        /*
        recebe pacote de dados buf1 = [D1...Dn]
        gera BCC2 = D1 ^ D2 ... ^ Dn
        - buf2 = [D1...Dn, BCC2]

        percorre buf2 e implementa stuffing
        - se encontrar 0x7E substitui por 0x7D 0x5E
        - se encontrar 0x7D substitui por 0x7D 0x5D

        cria trama I(0) ou I(1) 

        */

    int frameSize = 6 + bufSize;
    unsigned char *frame = (unsigned char*)malloc(frameSize);
    frame[0] = FLAG;
    frame[1] = A_SENDER;
    frame[2] = frameNumber;
    frame[3] = A_SENDER ^ frameNumber;
    memcpy(frame+4, buf, bufSize);
    
    unsigned char BCC2 = buf[0];
    for (unsigned int i = 1; i < bufSize; i++) {
        BCC2 ^= buf[i];
    }

    int j = 4;
    for (unsigned int i = 0; i < bufSize; i++) {
        if (buf[i] == FLAG) {
            frame = realloc(frame, frameSize+1);
            frame[j++] = ESC;
            frame[j++] = FLAG ^ STUFF;
        }
        else if (buf[i] == ESC) {
            frame = realloc(frame, frameSize+1);
            frame[j++] = ESC;
            frame[j++] = ESC ^ STUFF;
        }
        else {
            frame[j++] = buf[i];
        }
    }

    frame[j++] = BCC2;
    frame[j] = FLAG;

    alarmEnabled = FALSE;
    alarmCount = 0;
    int n_bytes_received = 0;

    unsigned char buf_rc[BUF_SIZE];

    STOP = FALSE;


    while (STOP == FALSE && alarmCount < nRetransmissions)
    {
        if (alarmEnabled == FALSE) {
            int bytes = writeBytesSerialPort(frame, frameSize);
            printf("%d bytes written\n", bytes);
            sleep(1);
            setAlarm(timeout);
        }

        s = START;
        n_bytes_received = 0;

        while (n_bytes_received <= 5 && s == STOP_RCV)
        {
            int bytes = readByteSerialPort(buf_rc);
            n_bytes_received++;
            printf("Bytes received: %d\n", bytes);
            if (bytes > 0)
                state_machine_transmitter(buf_rc[0]);
        }

        if (s = STOP_RCV) {
            STOP = TRUE;
        }
    }
    if (alarmCount >= nRetransmissions) {
        return 0;
    }
    return 1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    int clstat = closeSerialPort();
    return clstat;
}
