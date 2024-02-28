#include "RUDP_API.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h> // For gettimeofday()
#define TRUE 1
#define FALSE 0
#define COMMAND "COMMAND x"
#define RECEIVE_COMMAND "COMMAND 1"
#define EXIT_COMMAND "COMMAND 2"
#define FILE_SIZE 3532695

/*
 * @brief The maximum number of clients that the server can handle.
 * @note The default maximum number of clients is 1.
 */
#define MAX_CLIENTS 1

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
 */
#define BUFFER_SIZE 1024
typedef struct TIME_AND_BANDWIDTH
{
    double duration;
    double bandwidth;
    struct TIME_AND_BANDWIDTH *next;
} TIME_AND_BANDWIDTH;

void usage()
{
    printf("Usage: ./RUDP_Receiver -p PORT\n");
    printf("PORT - The RUDP port of the Receiver.\n");
}
int get_command(int client_sock)
{
    int command = 0;
    char command_buffer[sizeof(COMMAND)];
    for (int i = 0; i < sizeof(COMMAND); i++)
    {
        rudp_recv(client_sock, &command_buffer[i]);
    }
    if (strcmp(command_buffer, EXIT_COMMAND) == 0)
    {
        printf("Received exit command\n");
        command = 0;
    }
    else if (strcmp(command_buffer, RECEIVE_COMMAND) == 0)
    {
        command = 1;
    }

    return command;
}
// Function to get current time in milliseconds
// long getCurrentTime()
// {
//     struct timeval te;
//     gettimeofday(&te, NULL);                                    // Get current time
//     long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // Calculate milliseconds
//     return milliseconds;
// }

int main(int argc, char *argv[])
{
    TIME_AND_BANDWIDTH *time_and_bandwidth_list = NULL, *last_time_and_bandwidth = NULL;
    char *buffer = (char *)malloc(FILE_SIZE);
    int port = -1;
    if ((argc != 3) ||
        !(strcmp(argv[1], "-p") == 0) ||
        (port = atoi(argv[2])) <= 0)
    {
        usage();
        exit(-1);
    }

    printf("port = %d\n", port);
    printf("Starting Receiver...\n");

    int sock = -1;

    // The variable to store the server's address.
    struct sockaddr_in server;

    // The variable to store the client's address.
    struct sockaddr_in client;

    // Stores the client's structure length.
    socklen_t client_len = sizeof(client);

    // The variable to store the socket option for reusing the server's address.
    int opt = 1;

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    sock = create_rudp_socket(port);
    if (sock == -1)
    {
        perror("Failed to create RUDP socket");
        return 1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(2)");
        rudp_close(sock);
        return 1;
    }
     // Set the server's address to "0.0.0.0" (all IP addresses on the local machine).
    server.sin_addr.s_addr = INADDR_ANY;

    // Set the server's address family to AF_INET (IPv4).
    server.sin_family = AF_INET;

    // Set the server's port to the specified port. Note that the port must be in network byte order.
    server.sin_port = htons(port);



     printf("Waiting for RUDP connection...\n");



    // struct sockaddr_in sender_addr;
    // char buffer[BUFFER_SIZE];
    // long startTime, endTime;
    // int totalBytesReceived = 0, runCount = 1;
    // double totalTime = 0.0, totalSpeed = 0.0;

    // while (1)
    // {
    //     memset(buffer, 0, BUFFER_SIZE);
    //     int bytesReceived = rudp_recv(sockfd, buffer, BUFFER_SIZE, &sender_addr);
    //     if (bytesReceived <= 0)
    //         continue; // Skip any errors or empty transmissions

    //     if (runCount == 1)
    //     {
    //         printf("Connection request received, sending ACK.\n");
    //         startTime = getCurrentTime(); // Start timing on first valid packet
    //     }

    //     totalBytesReceived += bytesReceived;
    //     if (strcmp(buffer, "Fin") == 0)
    //     {
    //         endTime = getCurrentTime();                                          // End timing on receiving "Fin"
    //         double timeTaken = (endTime - startTime) / 1000.0;                   // Convert to seconds
    //         double speed = (totalBytesReceived / (1024.0 * 1024.0)) / timeTaken; // MB/s

    //         printf("File transfer completed.\nACK sent.\n");
    //         printf("----------------------------------\n");
    //         printf("- * Statistics *\n-\n");
    //         printf("- Run #%d Data: Time=%.2fms; Speed=%.2fMB/s\n-\n", runCount, timeTaken * 1000, speed);
    //         printf("----------------------------------\n");

    //         totalTime += timeTaken;
    //         totalSpeed += speed;
    //         totalBytesReceived = 0; // Reset for next run
    //         runCount++;

    //         // Wait for another "Fin" or program termination
    //         printf("Waiting for Sender response...\n");
    //         continue;
    //     }
    // }

    // if (runCount > 1)
    // {
    //     printf("Receiver end.\n");
    //     printf("----------------------------------\n");
    //     printf("- Average time: %.2fms\n", (totalTime / (runCount - 1)) * 1000);
    //     printf("- Average bandwidth: %.2fMB/s\n", totalSpeed / (runCount - 1));
    //     printf("----------------------------------\n");
    // }

    // close_rudp_socket(sockfd);
    return 0;
}
