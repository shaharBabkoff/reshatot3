#include <stdio.h>      // Standard input/output library
#include <stdlib.h>     // Standard library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <netinet/tcp.h>
#include <sys/time.h>
 #include "TCP.h"
// #define TRUE 1
// #define FALSE 0
// #define COMMAND "COMMAND x"
// #define RECEIVE_COMMAND "COMMAND 1"
// #define EXIT_COMMAND "COMMAND 2"
// #define FILE_SIZE 3532695

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
    printf("Usage: ./TCP_Receiver -p PORT -algo ALGO\n");
    printf("PORT - The TCP port of the Receiver.\n");
    printf("ALGO -  TCP congestion control algorithm that will be used by the party.\n");
}

int get_command(int client_sock)
{
    int command = 0;
    char command_buffer[sizeof(COMMAND)];
    for (int i = 0; i < sizeof(COMMAND); i++)
    {
        recv(client_sock, &command_buffer[i], 1, 0);
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
TIME_AND_BANDWIDTH *read_file(int client_sock, int file_size, char *buffer, int buffer_size)
{
    TIME_AND_BANDWIDTH *ret_val = NULL;
    struct timeval startTime, endTime;
    int bytes_received = 0;
    gettimeofday(&startTime, NULL);
    while (bytes_received < file_size)
    {
        int n;
        n = recv(client_sock, buffer + bytes_received, buffer_size - bytes_received, 0);
        bytes_received += n;
        if (n <= 0)
        {
            return ret_val;
        }
    }
    gettimeofday(&endTime, NULL);
    printf("bytes received: %d\n", bytes_received);
    ret_val = (TIME_AND_BANDWIDTH *)malloc(sizeof(TIME_AND_BANDWIDTH));
    // ret_val->duration = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
    ret_val->duration = ((endTime.tv_sec - startTime.tv_sec) * 1000.0) + ((endTime.tv_usec - startTime.tv_usec) / 1000.0); // Convert to milliseconds
    ret_val->bandwidth = (bytes_received / ret_val->duration / (1024.0 * 1024.0)) * 1000;                                           // Speed in MB/s
    ret_val->next = NULL;
    return ret_val;
}

int main(int argc, char *argv[])
{
    TIME_AND_BANDWIDTH *time_and_bandwidth_list = NULL, *last_time_and_bandwidth = NULL;
    char *buffer = (char *)malloc(FILE_SIZE);
    int port = -1;

    if ((argc != 5) ||
        !(strcmp(argv[1], "-p") == 0) ||
        !(strcmp(argv[3], "-algo") == 0) ||
        !((strcmp(argv[4], "reno") == 0) || (strcmp(argv[4], "cubic") == 0)) ||
        (port = atoi(argv[2])) <= 0)
    {
        usage();
        exit(-1);
    }

    printf("port = %d, algo = %s\n", port, argv[4]);
    printf("Starting Receiver...\n");

    // The variable to store the socket file descriptor.
    int sock = -1;

    // The variable to store the server's address.
    struct sockaddr_in server;

    // The variable to store the client's address.
    struct sockaddr_in client;

    // Stores the client's structure length.
    socklen_t client_len = sizeof(client);

    // The variable to store the socket option for reusing the server's address.
    int opt = 1;

    // Reset the server and client structures to zeros.
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }

    // Set the socket option to reuse the server's address.
    // This is useful to avoid the "Address already in use" error message when restarting the server.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(2)");
        close(sock);
        return 1;
    }

    // Set the server's address to "0.0.0.0" (all IP addresses on the local machine).
    server.sin_addr.s_addr = INADDR_ANY;

    // Set the server's address family to AF_INET (IPv4).
    server.sin_family = AF_INET;

    // Set the server's port to the specified port. Note that the port must be in network byte order.
    server.sin_port = htons(port);

    // Try to bind the socket to the server's address and port.
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind(2)");
        close(sock);
        return 1;
    }

    // Try to listen for incoming connections.
    if (listen(sock, MAX_CLIENTS) < 0)
    {
        perror("listen(2)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Waiting for TCP connections\n");

    // Try to accept a new client connection.
    int client_sock = accept(sock, (struct sockaddr *)&client, &client_len);

    // If the accept call failed, print an error message and return 1.
    if (client_sock < 0)
    {
        perror("accept(2)");
        close(sock);
        return 1;
    }
    if (setsockopt(client_sock, IPPROTO_TCP, TCP_CONGESTION, argv[4], strlen(argv[4])) != 0)
    {
        close(client_sock);
        close(sock);
        perror("setsockopt");
        return -1;
    }

    // Print a message to the standard output to indicate that a new client has connected.
    fprintf(stdout, "Sender connected, beginning to receive file...\n");
    printf("Sender connected, beginning to receive file...\n");
    while ((get_command(client_sock)) > 0)
    {
        // printf("file size is %d, going to read file\n", FILE_SIZE);
        TIME_AND_BANDWIDTH *tmp;
        if ((tmp = read_file(client_sock, FILE_SIZE, buffer, FILE_SIZE)) == NULL)
        {
            printf("socket dissconected, exiting\n");
            break;
        }
        if (time_and_bandwidth_list == NULL)
        {
            time_and_bandwidth_list = tmp;
            last_time_and_bandwidth = tmp;
        }
        else
        {
            last_time_and_bandwidth->next = tmp;
            last_time_and_bandwidth = tmp;
        }
        printf("Waiting for Sender response...\n");
    }
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    TIME_AND_BANDWIDTH *current = time_and_bandwidth_list;
    double sum_avarege = 0;
    double sum_bandwidth = 0;
    int index = 0;
    while (current != NULL)
    {
        index++;
        printf("- Run #%d Data: Time=%.6fms; Speed=%.6fMB/s\n", index, current->duration, current->bandwidth);
        sum_avarege += current->duration;
        sum_bandwidth += current->bandwidth;
        current = current->next;
    }
    printf("- Average time: %.2fms\n", sum_avarege / (double)index);
    printf("- Average bandwidth: %.2fMB/s\n", sum_bandwidth / (double)index);
    printf("----------------------------------\n");
    printf("Receiver end.\n");
    TIME_AND_BANDWIDTH *current2 = time_and_bandwidth_list, *temp;
    while (current2 != NULL)
    {
        temp = current2;
        current2 = current2->next;
        free(temp);
    }
    free(buffer);

    return 1;
}