#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include "TCP.h"

// #define BUFFER_SIZE 1024
// #define EXIT_TERMINAL "EXIT"
// #define FILE_SIZE 3532695

void usage()
{
    printf("Usage: ./TCP_Sender -ip IP -p PORT -algo ALGO\n");
    printf("PORT - The TCP port of the Sender.\n");
    printf("ALGO -  TCP congestion control algorithm that will be used by the party.\n");
}

int main(int argc, char *argv[])
{

    int port = -1;
    char *ip;

    if ((argc != 7) ||
        !(strcmp(argv[1], "-ip") == 0) ||
        !(strcmp(argv[3], "-p") == 0) ||
        !(strcmp(argv[5], "-algo") == 0) ||
        !((strcmp(argv[6], "reno") == 0) || (strcmp(argv[6], "cubic") == 0)) ||
        ((port = atoi(argv[4])) <= 0))
    {
        usage();
        exit(-1);
    }
    char *buffer = (char *)malloc(FILE_SIZE);
    FILE *fp;
    char *filename = "testFile";
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Error in reading file.");
        exit(-1);
    }
    fgets(buffer, FILE_SIZE, fp);
    ip = argv[2];
    printf("port = %d, algo = %s\n", port, argv[6]);

    printf("Starting Receiver...\n");
    // The variable to store the socket file descriptor
    int sock = -1;
    // The variable to store the Receiver's address.
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    // Try to create a TCP socket
    fprintf(stdout, "try to create TCP");
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, argv[6], strlen(argv[6])) != 0)
    {
        perror("setsockopt");
        return -1;
    }
    if (inet_pton(AF_INET, ip, &server.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }
    // Set the Receiver's address family to AF_INET (IPv4).
    server.sin_family = AF_INET;

    // Set the Receiver's port to the defined port. Note that the port must be in network byte order,
    // so we first convert it to network byte order using the htons function.
    server.sin_port = htons(port);

    fprintf(stdout, "Connecting to %s:%d...\n", ip, port);

    // Try to connect to the Receiver using the socket and the Receiver structure.
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect(2)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Successfully connected to the receiver !\n");

    char userChoise = '1';

    while (userChoise == '1')
    {

        send(sock, RECEIVE_COMMAND, 10, 0);
        send(sock, buffer, FILE_SIZE, 0);
        printf("please enter your choice: (1 for resending, any other character to quit)\n");
        scanf(" %c", &userChoise);
        printf("user choice %c\n", userChoise);
    }
    send(sock, EXIT_COMMAND, 10, 0);
    close(sock);
    fprintf(stdout, "Connection closed!\n");
    free(buffer);
    return 0;
}