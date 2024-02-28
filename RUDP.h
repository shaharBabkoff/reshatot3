#include <stdio.h>
#include <stdint.h>

#define WINDOW_SIZE 4
#define MAX_PAYLOAD_SIZE 1472
#define FILE_SIZE 21389444

 enum RUDP_Flag{
    DATA = 0x01,
    ACK = 0x02,
    SYN = 0x04,
    FIN = 0x08
} ;

typedef struct hdr {
    unsigned short length;
    unsigned short checksum;
    unsigned char flags;
    unsigned char seqnum;
};

#define MAX_PACKET_DATA (MAX_PAYLOAD_SIZE - sizeof(struct hdr))

typedef struct data
{
    struct hdr header;
    char data[MAX_PACKET_DATA]; 
};




