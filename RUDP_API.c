#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#define MAX_PKT_SIZE 1024
#define HEADER_SIZE sizeof(RUDP_Header)
#define DATA_SIZE (MAX_PKT_SIZE - HEADER_SIZE)
#define WINDOW_SIZE 4
#define TIMEOUT_SEC 2
#define MAX_SEQ_NUM 256

typedef enum {
    DATA = 0x01,
    ACK = 0x02,
    SYN = 0x04,
    FIN = 0x08
} RUDP_Flagg;

typedef struct {
    unsigned short length;
    unsigned short checksum;
    unsigned char flags;
    unsigned char seq_num;
} RUDP_Header;

typedef struct {
    RUDP_Header header;
    char data[DATA_SIZE];
} RUDP_Packet;

int sockfd;
struct sockaddr_in peer_addr;
socklen_t peer_addr_len = sizeof(peer_addr);
unsigned char expected_seq_num = 0;
unsigned char send_seq_num = 0;
static unsigned char next_seq_num = 0;


// Function Declarations
// int create_rudp_socket(int port);
// void rudp_close();
// int rudp_send(int sockfd, const struct sockaddr_in *address, const void *buf, size_t len);
// int rudp_recv(void *buf, size_t len);
// unsigned short calculate_checksum(const void *data, size_t len);
// void handle_timeout();
// int send_packet(const RUDP_Packet *pkt, size_t len);
// int receive_packet(RUDP_Packet *pkt, size_t *len);

// Create RUDP Socket
int create_rudp_socket(int port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Close RUDP Socket
void rudp_close() {
    close(sockfd);
}

// Send Data with RUDP
int rudp_send(int sockfd, const struct sockaddr_in *address, const void *buf, size_t len) {
    RUDP_Packet packet;
    memset(&packet, 0, sizeof(RUDP_Packet));

    // Populate the RUDP header
    packet.header.flags = DATA; // Assuming DATA is a flag defined for data packets
    packet.header.seq_num = next_seq_num++; // Assuming a global sequence number variable
    packet.header.length = htons(len);
    packet.header.checksum = htons(calculate_checksum(buf, len)); // Function to calculate checksum

    // Copy the data into the packet
    memcpy(packet.data, buf, len);

    // Send the packet over the UDP socket
    ssize_t sent_bytes = sendto(sockfd, &packet, HEADER_SIZE + len, 0, (const struct sockaddr *)address, sizeof(struct sockaddr_in));
    if (sent_bytes < 0) {
        perror("sendto failed");
        return -1;
    }

    return 0; // Success
}

// Receive Data with RUDP
int rudp_recv(void *buf, size_t len) {
    RUDP_Packet pkt;
    size_t pkt_len;

    while (1) {
        if (receive_packet(&pkt, &pkt_len) < 0) continue;
        if (pkt.header.flags & ACK) continue;  // Ignore ACK packets

        if (pkt.header.seq_num == expected_seq_num && pkt.header.flags & DATA) {
            memcpy(buf, pkt.data, pkt.header.length);
            expected_seq_num = (expected_seq_num + 1) % MAX_SEQ_NUM;

            // Send ACK
            RUDP_Packet ack_pkt;
            memset(&ack_pkt, 0, sizeof(ack_pkt));
            ack_pkt.header.flags = ACK;
            ack_pkt.header.seq_num = pkt.header.seq_num;
            send_packet(&ack_pkt, HEADER_SIZE);

            return pkt.header.length;
        }
    }
}

// Checksum Calculation (Simplified Version)
unsigned short calculate_checksum(const void *data, size_t len) {
    const unsigned short *buf = data;
    unsigned long sum = 0;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

// Send Packet
int send_packet(const RUDP_Packet *pkt, size_t len) {
    if (sendto(sockfd, pkt, len, 0, (struct sockaddr *)&peer_addr, peer_addr_len) != len) {
        perror("sendto");
        return -1;
    }
    return 0;
}

// Receive Packet
int receive_packet(RUDP_Packet *pkt, size_t *len) {
    *len = recvfrom(sockfd, pkt, MAX_PKT_SIZE, 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
    if (*len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Timeout
            handle_timeout();
        } else {
            perror("recvfrom");
        }
        return -1;
    }
    return 0;
}

// Handle Timeout (Simple Version)
void handle_timeout() {
    printf("Timeout occurred\n");
    // Implement timeout handling, e.g., retransmit packets
}
