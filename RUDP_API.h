#include <stdio.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAX_PKT_SIZE 1472
#define HEADER_SIZE sizeof(RUDP_Header)
#define MAX_DATA_SIZE (MAX_PKT_SIZE - HEADER_SIZE)
#define TIMEOUT_SEC 2
#define MAX_WAIT_TIME 500000
#define MAX_RETRANSMITS 1000


typedef enum
{
    DATA = 0x01,
    ACK = 0x02,
    SYN = 0x04,
    FIN = 0x08
} RUDP_Flag;

typedef struct
{
    unsigned short length; // length of the data not including the header itself
    unsigned short checksum; // calculated checksum of the entire packet (including the header while checksum is initialized to 0)
    unsigned char flags; // value describing the request. can have one of the following values:  SYN, ACK, SYN | ACK, DATA, FIN
    unsigned char udf; // user define meta data
} RUDP_Header;


// a struct to combine a header and data for purposes of calculating checksum and for calling sento/recvfrom
typedef struct
{
    RUDP_Header hdr; // header for the packet
    char data[MAX_DATA_SIZE]; // data of the packet
} RUDP_Packet;

// A struct that represents RUDP Socket
typedef struct _rudp_socket
{
    int socket_fd;                // UDP socket file descriptor
    bool isServer;                // True if the RUDP socket acts like a server, false for client.
    bool isConnected;             // True if there is an active connection, false otherwise.
    struct sockaddr_in dest_addr; // Destination address. Client fills it when it connects via rudp_connect(), server fills it when it accepts a connection via rudp_accept().
} RUDP_Socket;

// Allocates a new structure for the RUDP socket (contains basic information about the socket itself).
// Also creates a UDP socket as a baseline for the RUDP. isServer means that this socket acts like a server.
// If set to server socket, it also binds the socket to a specific port.
RUDP_Socket *rudp_socket(bool isServer, unsigned short int listen_port);

// Tries to connect to the other side via RUDP to given IP and port. Returns 0 on failure and 1 on success.
// Fails if called when the socket is connected/set to server.
int rudp_connect(RUDP_Socket *sockfd, const char *dest_ip, unsigned short int dest_port);

// Accepts incoming connection request and completes the handshake, returns 0 on failure and 1 on success.
// Fails if called when the socket is connected/set to client.
int rudp_accept(RUDP_Socket *sockfd);

// Receives data from the other side and put it into the buffer. Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_recv(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size, unsigned char *udf);

// Sends data stores in buffer to the other side. Returns the number of sent bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size, unsigned char udf);

// Disconnects from an actively connected socket. Returns 1 on success, 0 when the socket is already disconnected (failure).
int rudp_disconnect(RUDP_Socket *sockfd);

// This function releases all the memory allocation and resources of the socket.
int rudp_close(RUDP_Socket *sockfd);
