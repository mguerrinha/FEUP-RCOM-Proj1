// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define DATA_SIZE 256

#include "manage_packet.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    LinkLayerRole linkLayerRole;

    if (strcmp(role, "tx") == 0) {
        linkLayerRole = LlTx;       
    }
    else if (strcmp(role, "rx") == 0) {
        linkLayerRole = LlRx;
    }
    else {
        printf("Wrong role\n");
    }

    LinkLayer connectionParam;

    strcpy(connectionParam.serialPort, serialPort);
    connectionParam.role = linkLayerRole;
    connectionParam.baudRate = baudRate;
    connectionParam.nRetransmissions = nTries;
    connectionParam.timeout = timeout;

    printf("LLOPEN\n");
        
    if (llopen(connectionParam) == -1) {
        perror("Erro: Connection Failed\n");
        exit(-1);
    }

    printf("Connection Established\n");
    FILE* file;
    FILE* newFile;
    unsigned char *packet;
    int STOP;

    switch (connectionParam.role)
    {
    case LlTx:
        file = fopen(filename, "rb");

        if (file == NULL) {
            perror("File not found\n");
            exit(-1);
        }

        struct stat fileStat;

        if (stat(filename, &fileStat) == -1) {
            perror("Error: Could not retrieve file information\n");
            exit(-1);
        }
        
        printf("File size: %ld bytes\n", fileStat.st_size);

        unsigned int bufSize;

        unsigned char *controlStart = getControlPacket(1, filename, fileStat.st_size, &bufSize);

        if (llwrite(controlStart, bufSize) == -1) {
            perror("Error: Start packet error\n");
            exit(-1);
        }

        unsigned int sequence = 0;
        unsigned char *content = getData(file, fileStat.st_size);
        size_t bytesLeft = fileStat.st_size;

        while (bytesLeft > 0)
        {
            int dataSize = bytesLeft > (size_t) DATA_SIZE ? DATA_SIZE : bytesLeft;
            printf("--------------------\n");
            printf("Bytes Left %ld de %ld\n", bytesLeft, fileStat.st_size);
            printf("--------------------\n");
            unsigned char *data = (unsigned char *) malloc(dataSize);
            memcpy(data, content, dataSize);
            unsigned int packetSize;
            unsigned char *packet = getPacketData(sequence, data, dataSize, &packetSize);

            if (llwrite(packet, packetSize) == -1) {
                perror("Error: error in data packets\n");
                exit(-1);
            }

            bytesLeft -= dataSize;
            content += dataSize;
            sequence = (sequence + 1) % 100;
        }
        
        unsigned char *controlEnd = getControlPacket(3, filename, fileStat.st_size, &bufSize);
        printf("END PACKET\n");
        if (llwrite(controlEnd, bufSize) == -1) {
            perror("Error: End packet error\n");
            exit(-1);
        }

        int close = llclose(1);

        if (close == -1) {
            perror ("Error closing serial port\n");
            exit(-1);
        }

        if (close == 1) {
            perror("End connection failed\n");
            exit(-1);
        }
        break;    
    case LlRx:
        
        packet = (unsigned char *) malloc(DATA_SIZE+4);
        int packetSize = -1;
        STOP = FALSE;

        while (!STOP)
        {
            packetSize = -1;
            packetSize = llread(packet);
            if (packetSize > 0 && packet[0] == 1) {
                printf ("START PACKET RCV\n");
                STOP = TRUE;
            }
        }

        size_t fileSize = parseControlPacket(packet);
        size_t totalReceived = 0;

        newFile = fopen((char *) filename, "wb+");
        STOP = FALSE;
        int sequenceConfirm = -1;

        while (!STOP)
        {
            packetSize = -1;
            packetSize = llread(packet);

            if (packet[0] == 3) {
                printf("END PACKET RCV\n");
                if (llclose(1) == -1) {
                    perror ("Error closing serial port\n");
                    exit(-1);
                }
                STOP = TRUE;
            }
            else if (packetSize > 0 && packet[0] == 2) {
                totalReceived += packetSize-4;
                printf("----------------\n");
                printf("Progression %ld/%ld\n", totalReceived, fileSize);
                printf("----------------\n");
                if (packet[1] == sequenceConfirm) {
                    perror("Error duplicated frame received");
                    exit(-1);
                }
                sequenceConfirm = packet[1];
                fwrite(packet+4, sizeof(unsigned char), packetSize-4, newFile);
            }
            else continue;
        }

        fclose(newFile);
        break;

    default:
        exit(-1);
        break;
    }

}
