#ifndef MANAGE_PACKET_H
#define MANAGE_PACKET_H

#include <stdio.h>
#include <stdlib.h>

unsigned char *getControlPacket(const unsigned int control, const char *filename, unsigned int filesize, int bufSize);

unsigned char *getData(FILE *file, size_t fileSize);

unsigned char *getPacketData(unsigned int sequence, unsigned char *data, int dataSize, int *packetSize);
#endif