#ifndef MANAGE_PACKET_H
#define MANAGE_PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *getControlPacket(const unsigned int control, const char *filename, unsigned int filesize, unsigned int *bufSize);

unsigned char *getData(FILE *file, size_t fileSize);

unsigned char *getPacketData(unsigned int sequence, unsigned char *data, unsigned int dataSize, unsigned int *packetSize);

size_t parseControlPacket(unsigned char *packet);

void parseData(unsigned char *packet, int packetSize, unsigned char *buffer);

#endif