#include "manage_packet.h"

// Construction of a control Packet
unsigned char *getControlPacket(const unsigned int control, const char *filename, unsigned int filesize, unsigned int *bufSize) {
    int L1;
    unsigned int numBits = 0;
    unsigned int auxFileSize = filesize;

    if (auxFileSize == 0) {
        L1 = 1;
    }

    while (auxFileSize > 0)
    {
        numBits++;
        auxFileSize >>= 1;
    }

    L1 = (numBits + 7) / 8;
    
    int L2 = strlen(filename);
    *bufSize = 5 + L1 + L2;

    unsigned char *packet = (unsigned char*) malloc(*bufSize);

    unsigned int pos = 0;

    packet[pos++] = control;
    packet[pos++] = 0;
    packet[pos++] = L1;

    for (unsigned int i = 0; i < L1; i++) {
        packet[2+L1-i] = filesize & 0xFF;
        filesize >>=8;
    }

    pos += L1;
    packet[pos++] = 1;
    packet[pos++] = L2;
    memcpy(packet+pos, filename, L2);

    return packet;
}

// Get all the data from the file to be transfered
unsigned char *getData(FILE *file, size_t fileSize) {
    unsigned char *content = (unsigned char *) malloc (fileSize);

    size_t bytesRead = fread(content, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror ("Error reading file");
        free(content);
        return NULL;
    }

    return content;
}

// Construction of the data Packet
unsigned char *getPacketData(unsigned int sequence, unsigned char *data, unsigned int dataSize, unsigned int *packetSize) {
    *packetSize = 4 + dataSize;
    unsigned char *packet = (unsigned char *) malloc(*packetSize);

    packet[0] = 2;
    packet[1] = sequence;
    packet[2] = dataSize >> 8 & 0xFF;
    packet[3] = dataSize & 0xFF;
    memcpy(packet+4, data, dataSize);

    return packet;
}

// Parse the control Packet and take all the necessary information 
size_t parseControlPacket(unsigned char *packet) {
    unsigned char nFileSizeBytes = packet[2];
    unsigned char fileSizeAux[nFileSizeBytes];
    memcpy(fileSizeAux, packet+3, nFileSizeBytes);

    size_t fileSize = 0;
    for (unsigned int i = 0; i < nFileSizeBytes; i++) {
        fileSize |= (fileSizeAux[nFileSizeBytes-i-1] << (8*i));
    }
    return fileSize;
}
