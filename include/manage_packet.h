#ifndef MANAGE_PACKET_H
#define MANAGE_PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Construction of a control Packet
unsigned char *getControlPacket(const unsigned int control, const char *filename, unsigned int filesize, unsigned int *bufSize);

// Get all the data from the file to be transfered
unsigned char *getData(FILE *file, size_t fileSize);

// Construction of the data Packet
unsigned char *getPacketData(unsigned int sequence, unsigned char *data, unsigned int dataSize, unsigned int *packetSize);

// Parse the control Packet and take all the necessary information 
size_t parseControlPacket(unsigned char *packet);

#endif