#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "RUDP_API.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

unsigned short int calculate_checksum(void *data, unsigned int bytes)
{
    unsigned short int *data_pointer = (unsigned short int *)data;
    unsigned int total_sum = 0;
    // Main summing loop
    while (bytes > 1)
    {
        total_sum += *data_pointer++;
        bytes -= 2;
    }
    // Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char *)data_pointer);
    // Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);
    return (~((unsigned short int)total_sum));
}
// Allocates a new structure for the RUDP socket (contains basic information about the socket itself).
// Also creates a UDP socket as a baseline for the RUDP. isServer means that this socket acts like a server.
// If set to server socket, it also binds the socket to a specific port.

RUDP_Socket *rudp_socket(bool isServer, unsigned short int listen_port)
{
    RUDP_Socket *rudp_sock = (RUDP_Socket *)malloc(sizeof(RUDP_Socket));
    int sock = -1;
    int opt = 1;
    // The variable to store the server's address.
    memset(&rudp_sock->dest_addr, 0, sizeof(rudp_sock->dest_addr));

    // The variable to store the client's address.
    // struct sockaddr_in client;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket(2)");
        free(rudp_sock);
        return NULL;
    }

    if (isServer)
    {
        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            perror("setsockopt(2)");
            close(sock);
            free(rudp_sock);
            return NULL;
        }
        printf("after setsockopt SO_REUSEADDR\n");
        // Set the server's address to "0.0.0.0" (all IP addresses on the local machine).
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_family = AF_INET;
        server.sin_port = htons(listen_port);

        // Set the server's address family to AF_INET (IPv4).

        // Try to bind the socket to the server's address and port.
        if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("bind(2)");
            close(sock);
            free(rudp_sock);
            return NULL;
        }
        printf("after bind\n");
    }
    else
    {
        struct timeval tv = {0, MAX_WAIT_TIME};
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) == -1)
        {
            perror("setsockopt(2)");
            close(sock);
            free(rudp_sock);
            return NULL;
        }
    }

    rudp_sock->isServer = isServer;
    rudp_sock->socket_fd = sock;
    rudp_sock->isConnected = false;
    printf("done creating socket\n");
    return rudp_sock;
}

// Tries to connect to the other side via RUDP to given IP and port. Returns 0 on failure and 1 on success.
// Fails if called when the socket is connected/set to server.

int rudp_connect(RUDP_Socket *sockfd, const char *dest_ip, unsigned short int dest_port)
{
    if (sockfd->isServer)
    {
        return 0;
    }
    printf("trying to connect...%s:%d\n", dest_ip, dest_port);
    // struct sockaddr_in server;
    memset(&sockfd->dest_addr, 0, sizeof(sockfd->dest_addr));
    if (inet_pton(AF_INET, dest_ip, &sockfd->dest_addr.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sockfd->socket_fd);
        return 0;
    }

    sockfd->dest_addr.sin_family = AF_INET;
    sockfd->dest_addr.sin_port = htons(dest_port);
    // sockfd->dest_addr = server;
    RUDP_Header hdr;
    hdr.checksum = 0;
    hdr.flags = SYN;
    hdr.length = 0;
    hdr.udf = 0;
    hdr.checksum = calculate_checksum(&hdr, sizeof(hdr));
    printf("sending SYN\n");
    int bytes_sent = sendto(sockfd->socket_fd, &hdr, sizeof(hdr), 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (bytes_sent <= 0)
    {
        perror("sendto(2)");
        close(sockfd->socket_fd);
        return 0;
    }
    printf("sent %d bytes\n", bytes_sent);
    struct sockaddr_in recv_server;
    socklen_t recv_server_len = sizeof(recv_server);
    printf("calling recvfrom\n");
    int bytes_received = recvfrom(sockfd->socket_fd, &hdr, sizeof(hdr), 0, (struct sockaddr *)&recv_server, &recv_server_len);
    if (bytes_received <= 0)
    {
        printf("failed to recvfrom\n");
        perror("recvfrom(2)");
        close(sockfd->socket_fd);
        return 0;
    }
    unsigned short temp = hdr.checksum;
    hdr.checksum = 0;
    if (calculate_checksum(&hdr, sizeof(hdr)) != temp)
    {
        printf("checksumc failed\n");
        perror("checksum");
        close(sockfd->socket_fd);
        return 0;
    }
    if (hdr.flags != (SYN | ACK))
    {
        printf("no SYN | ACK\n");
        perror("NACK");
        close(sockfd->socket_fd);
        return 0;
    }
    printf("received SYN | ACK\n");
    sockfd->isConnected = true;
    return 1;
}

// Accepts incoming connection request and completes the handshake, returns 0 on failure and 1 on success.
// Fails if called when the socket is connected/set to client.

int rudp_accept(RUDP_Socket *sockfd)
{
    if (!sockfd->isServer || sockfd->isConnected)
    {
        return 0;
    }
    printf("in accept\n");
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    memset(&client, 0, sizeof(client));
    RUDP_Header hdr;
    printf("calling recvfrom\n");
    int bytes_received = recvfrom(sockfd->socket_fd, &hdr, sizeof(hdr), 0, (struct sockaddr *)&client, &client_len);
    printf("recvfrom returned %d\n", bytes_received);
    if (bytes_received <= 0)
    {
        printf("failed recvfrom\n");
        perror("recvfrom(2)");
        close(sockfd->socket_fd);
        return 0;
    }
    unsigned short temp = hdr.checksum;
    hdr.checksum = 0;
    if (calculate_checksum(&hdr, sizeof(hdr)) != temp)
    {
        printf("checksum failed\n");
        perror("checksum");
        close(sockfd->socket_fd);
        return 0;
    }
    if (hdr.flags != SYN)
    {
        printf("not SYN\n");
        perror("NACK");
        close(sockfd->socket_fd);
        return 0;
    }
    printf("received SYN\n");
    RUDP_Header syn_ack_hdr;
    syn_ack_hdr.flags = SYN | ACK;
    syn_ack_hdr.checksum = 0;
    syn_ack_hdr.checksum = calculate_checksum(&syn_ack_hdr, sizeof(syn_ack_hdr));
    sendto(sockfd->socket_fd, &syn_ack_hdr, sizeof(syn_ack_hdr), 0, (struct sockaddr *)&client, sizeof(client));
    sockfd->isConnected = true;
    sockfd->dest_addr = client;
    return 1;
}

// Receives data from the other side and puts it into the buffer.
// Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error.
// Fails if called when the socket is disconnected.

int rudp_recv(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size, unsigned char *udf)
{
    int ret_val = 0;
    if (!sockfd->isConnected)
    {
        printf("rudp_recv: socket is disconnected\n");
        return -1;
    }
    if (buffer_size > MAX_DATA_SIZE)
    {
        printf("rudp_recv: buffer too large\n");
        return -1;
    }
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    memset(&client, 0, sizeof(client));
    RUDP_Packet packet;
    int bytes_received = recvfrom(sockfd->socket_fd, &packet, sizeof(packet), 0, (struct sockaddr *)&client, &client_len);
    if (bytes_received <= 0)
    {
        printf("rudp_recv: bytes received %d\n", bytes_received);
        perror("recvfrom(2)");
        close(sockfd->socket_fd);
        sockfd->isConnected = false;
        return 0;
    }
    // printf("rudp_recv received %d bytes. length=%d, checksum=%d, flags=%d, udf=%d\n",
    //        bytes_received,
    //        packet.hdr.length,
    //        packet.hdr.checksum,
    //        packet.hdr.flags,
    //        packet.hdr.udf);
    unsigned short temp = packet.hdr.checksum;
    packet.hdr.checksum = 0;
    if (calculate_checksum(&packet, bytes_received) != temp)
    {
        printf("rudp_recv: checksum failed\n");
        perror("checksum");
        return -1;
    }

    RUDP_Header hdr;
    hdr.flags = ACK;
    hdr.length = 0;
    hdr.udf = 0;
    hdr.checksum = 0;
    hdr.checksum = calculate_checksum(&hdr, sizeof(hdr));
    sendto(sockfd->socket_fd, &hdr, sizeof(hdr), 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));

    if (packet.hdr.flags == FIN)
    {
        printf("rudp_recv received FIN, closing socket\n");
        close(sockfd->socket_fd);
        sockfd->isConnected = false;
        ret_val = 0;
    }
    else
    {
        memcpy(buffer, packet.data, min(buffer_size, packet.hdr.length));
        ret_val = packet.hdr.length;
    }
    if (udf != NULL)
    {
        *udf = packet.hdr.udf;
    }

    return ret_val;
}

// Sends data stored in buffer to the other side.
// Returns the number of sent bytes on success, 0 if got FIN packet (disconnect),
// and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size, unsigned char udf)
{

    int ret_val = -1;
    if (!sockfd->isConnected)
    {
        return -1;
    }
    if (buffer_size > MAX_DATA_SIZE)
    {
        return -1;
    }
    RUDP_Packet packet;
    packet.hdr.flags = DATA;
    packet.hdr.length = buffer_size;
    packet.hdr.udf = udf;
    memcpy(packet.data, buffer, buffer_size);
    packet.hdr.checksum = 0;
    packet.hdr.checksum = calculate_checksum(&packet, buffer_size + sizeof(RUDP_Header));
    struct sockaddr_in recv_server;
    socklen_t recv_server_len = sizeof(recv_server);

    int counter = 0;
    while (counter < MAX_RETRANSMITS)
    {
        int bytes_sent = sendto(sockfd->socket_fd, &packet, buffer_size + sizeof(RUDP_Header), 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
        if (bytes_sent <= 0)
        {
            perror("sendto(2)");
            rudp_disconnect(sockfd);
            return -1;
        }
        RUDP_Header hdr;
        int bytes_received = recvfrom(sockfd->socket_fd, &hdr, sizeof(hdr), 0, (struct sockaddr *)&recv_server, &recv_server_len);
        if (bytes_received < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("received timeout, incrementing counter=%d\n", counter);
                counter++;
                continue;
            }
            else
            {
                perror("recvfrom(2)");
                rudp_disconnect(sockfd);
                return -1;
            }
        }
        else if (bytes_received == 0)
        {
            perror("recvfrom(2)");
            rudp_disconnect(sockfd);
            return -1;
        }
        else
        {
            unsigned short temp = hdr.checksum;
            hdr.checksum = 0;
            if (calculate_checksum(&hdr, sizeof(hdr)) != temp)
            {
                printf("checksum failed!!");
                perror("checksum");
                counter++;
                continue;
            }
            if (hdr.flags != FIN && hdr.flags != ACK)
            {
                printf("invalid flags!!!");
                perror("NACK");
                ret_val = -1;
            }
            else if (hdr.flags == FIN)
            {
                ret_val = 0;
            }
            else
            {
                ret_val = buffer_size;
            }
            break;
        }
    }
    return ret_val;
}

// Disconnects from an actively connected socket.
// Returns 1 on success, 0 when the socket is already disconnected (failure).
int rudp_disconnect(RUDP_Socket *sockfd)
{

    if (sockfd->isConnected)
    {
        printf("rudp_disconnect sending FIN\n");
        RUDP_Header hdr;
        hdr.checksum = 0;
        hdr.flags = FIN;
        hdr.length = 0;
        hdr.checksum = calculate_checksum(&hdr, sizeof(hdr));
        hdr.udf = 0;
        sendto(sockfd->socket_fd, &hdr, sizeof(hdr), 0, (struct sockaddr *)&(sockfd->dest_addr), sizeof(sockfd->dest_addr));
        close(sockfd->socket_fd);
        sockfd->isConnected = false;
        return 1;
    }
    else
    {
        printf("rudp_disconnect doing nothing\n");
        return 0;
    }
}

// This function releases all the memory allocation and resources of the socket.
int rudp_close(RUDP_Socket *sockfd)
{
    printf("rudp_close\n");
    rudp_disconnect(sockfd);
    free(sockfd);
    return 0;
}
