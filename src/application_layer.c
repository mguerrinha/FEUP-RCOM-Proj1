// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "manage_packet.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
        // TODO
        /*
        - criar estrutura "connectionParam" do tipo LinkLayer com os valores de 5 parâmetros que recebeu (não passa filename)
        - invoca llopen passando o "connectionParam"

        - se Tx
        -- cria pacote START
        -- invoca llwrite passando START
        -- abre ficheiro "filename" em modo de leitura
        -- enquanto não chega ao fim do ficheiro:
        --- lê segmento de k bytes
        --- cria pacote de dados
        --- invoca llwrite passando pacote de dados
        -- cria pacote END e invoca llwrite

        - se Rx
        -- enquanto não recebe pacote END:
        --- invoca llread

        - invoca llclose
        */

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
    
    int connect = llopen(connectionParam);
    
    if (connect == 0) {
        perror("Erro: Connection Failed\n");
        exit(-1);
    }

    printf("Connection Established\n");

    switch (connectionParam.role)
    {
    case LlTx:
        FILE* file = fopen(filename, "rb");

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

        if (llwrite(controlStart, bufSize) == 0) {
            perror("Error: Start packet error\n");
            exit(-1);
        }

        unsigned int sequence = 0;
        unsigned char *content = getData(file, fileStat.st_size);
        size_t bytesLeft = fileStat.st_size;

        while (bytesLeft >= 0)
        {
            int dataSize = bytesLeft > (size_t) MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : bytesLeft;
            unsigned char *data = (unsigned char *) malloc(dataSize);
            memcpy(data, content, dataSize);
            unsigned int packetSize;
            unsigned char *packet = getPacketData(sequence, data, dataSize, &packetSize);

            if (llwrite(packet, packetSize) == -1) {
                perror("Error: error in data packets\n");
                exit(-1);
            }

            bytesLeft -= (size_t) MAX_PAYLOAD_SIZE;
            content += dataSize;
            sequence = (sequence + 1) % 100;
        }
        
        unsigned char *controlEnd = getControlPacket(3, filename, fileStat.st_size, &bufSize);
        if (llwrite(controlEnd, bufSize) == -1) {
            perror("Error: End packet error\n");
            exit(-1);
        }

        llclose(1);
        break;
    
    case LlRx:
        unsigned char *packet = (unsigned char *) malloc(MAX_PAYLOAD_SIZE);
        
        break;

    default:
        break;
    }

}
