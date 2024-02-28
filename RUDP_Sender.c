//#include "RUDP_API.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "RUDP.h"

// #define RECEIVER_IP "127.0.0.1"  // IP address of the receiver
// #define SERVER_PORT 5061         // Port number the receiver is listening on
#define BUFFER_SIZE 1024 // Size of the buffer for sending data
#define FILE_SIZE 3532695

void usage()
{
    printf("Usage: ./RUDP_Sender -ip IP -p PORT\n");
    printf("PORT - The RUDP port of the Sender.\n");
}

int main(int argc, char *argv[])
{

    int port = -1;
    char *ip;

    if ((argc != 5) ||
        !(strcmp(argv[1], "-ip") == 0) ||
        !(strcmp(argv[3], "-p") == 0) ||
        ((port = atoi(argv[4])) <= 0))
    {
        usage();
        exit(-1);
    }

    int array_size = FILE_SIZE / MAX_PACKET_DATA;
    if (FILE_SIZE % MAX_PACKET_DATA != 0)
    {
        array_size++;
    }
    struct data *data_array = (struct data *)malloc(array_size * sizeof(struct data));

    //char *buffer = (char *)malloc(FILE_SIZE);
    FILE *fp;
    char *filename = "testFile";
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error in reading file.");
        exit(1);
    }
    enum RUDP_Flag flag = DATA;
    for (int i = 0; i < array_size; i++)
    {
        int bytes_read = fread(data_array[i].data, 1, MAX_PACKET_DATA, fp);
        if (!ferror(fp))
        {
            data_array[i].header.length = bytes_read;
            data_array[i].header.seqnum = (unsigned char)(i % WINDOW_SIZE);
            data_array[i].header.flags = flag;
            //checksum missing
        }
    }
    for(int i = 0 ; i < MAX_PACKET_DATA;i++){
    printf("%c", data_array[3].data[i]);

    }
    

//    // fgets(buffer, FILE_SIZE, fp);
//     ip = argv[2];
//     printf("port = %d\n", port);

//     printf("Starting Receiver...\n");
//     // The variable to store the socket file descriptor
//     int sock = -1;
//     // The variable to store the Receiver's address.
//     struct sockaddr_in server;
//     memset(&server, 0, sizeof(server));
//     // Try to create a TCP socket
//     fprintf(stdout, "try to create RUDP");

//     sock = create_rudp_socket(0); // Create a socket with an ephemeral port
//     if (sock == -1)
//     {
//         perror("socket(2)");
//         return 1; // Exit with error code 1
//     }
//     if (inet_pton(AF_INET, ip, &server.sin_addr) <= 0)
//     {
//         perror("inet_pton(3)");
//         close_rudp_socket(sock);
//         return 1;
//     }
//     server.sin_family = AF_INET;
//     server.sin_port = htons(port);
//     fprintf(stdout, "Connecting to %s:%d...\n", ip, port);
//     char userChoise = '1';

//     while (userChoise == '1')
//     {

//         rudp_send(sock, "COMMAND 1", 10, 0);
//         //rudp_send(sock, buffer, FILE_SIZE, 0);
//         printf("please enter your choice: (1 for resending, any other character to quit)\n");
//         scanf(" %c", &userChoise);
//         printf("user choice %c\n", userChoise);
//     }
//     rudp_send(sock, "COMMAND 2", 10, 0);
//     rudp_close(sock);
//     fprintf(stdout, "Connection closed!\n");












    // Handshake process: SYN+ACK followed by SYN
    // char syn_ack[] = "Syn+Ack";
    // if (rudp_send(sockfd, &server_addr, syn_ack, sizeof(syn_ack)) < 0)
    // {
    //     perror("Handshake failed at SYN+ACK");
    //     close_rudp_socket(sockfd);
    //     return 1; // Exit with error code 1
    // }

    // char syn[] = "Syn";
    // if (rudp_send(sockfd, &server_addr, syn, sizeof(syn)) < 0)
    // {
    //     perror("Handshake failed at SYN");
    //     close_rudp_socket(sockfd);
    //     return 1; // Exit with error code 1
    // }

    // // Send data
    // char buffer[BUFFER_SIZE];
    // memset(buffer, 'A', BUFFER_SIZE); // Fill the buffer with dummy data for illustration

    // if (rudp_send(sockfd, &server_addr, buffer, BUFFER_SIZE) < 0)
    // {
    //     perror("Failed to send data");
    //     close_rudp_socket(sockfd);
    //     return 1; // Exit with error code 1
    // }

    // // Closing the connection: FIN+ACK followed by FIN
    // char fin_ack[] = "Fin+Ack";
    // if (rudp_send(sockfd, &server_addr, fin_ack, sizeof(fin_ack)) < 0)
    // {
    //     perror("Closing failed at FIN+ACK");
    // }

    // char fin[] = "Fin";
    // if (rudp_send(sockfd, &server_addr, fin, sizeof(fin)) < 0)
    // {
    //     perror("Closing failed at FIN");
    // }

    // close_rudp_socket(sockfd); // Close the socket before exiting
    return 0; // Exit successfully
}
