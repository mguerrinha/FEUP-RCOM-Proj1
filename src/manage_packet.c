#include "manage_packet.h"

unsigned char *getControlPacket(const unsigned int control, const char *filename, unsigned int filesize, unsigned int *bufSize) {
    int L1 = (int) ceil(log2f((float)filesize)/8.0);
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

unsigned char *getData(FILE *file, size_t fileSize) {
    unsigned char *content = (unsigned char *) malloc (fileSize);
    if (content == NULL) {
        perror("Memory allocation failed\n");
        return NULL;
    }

    size_t bytesRead = fread(content, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror ("Error reading file");
        free(content);
        return NULL;
    }

    return content;
}

unsigned char *getPacketData(unsigned int sequence, unsigned char *data, int dataSize, int *packetSize) {
    *packetSize = 4 + dataSize;
    unsigned char *packet = (unsigned char *) malloc(*packetSize);

    packet[0] = 2;
    packet[1] = sequence;
    packet[2] = dataSize >> 8 & 0xFF;
    packet[3] = dataSize & 0xFF;
    memcpy(packet+4, data, dataSize);

    return packet;
}
