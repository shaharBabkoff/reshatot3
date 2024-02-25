#include <stdio.h>      // Standard input/output library
#include <stdlib.h>     // Standard library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <time.h>
#include <netinet/tcp.h>

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
    printf("Usage: ./TCP_Receiver -p PORT -algo ALGO\n");
    printf("PORT - The TCP port of the Receiver.\n");
    printf("ALGO -  TCP congestion control algorithm that will be used by the party.\n");
}

int get_command(int client_sock)
{
    int command = 0;
    char command_buffer[sizeof(COMMAND)];
    // int bytes_read = 0;
    // int size_received = 0;

    // size_buffer can contain a string representation of an int which can be up to 11 characters (including '\0')
    // char size_buffer[11] = "";
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
    time_t startTime, endTime;
    // time(&startTime);
    startTime = time(0);
    int bytes_received = 0;

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
    //time(&endTime);
    endTime = time(0);
    printf("bytes received: %d\n", bytes_received);
    ret_val = (TIME_AND_BANDWIDTH *)malloc(sizeof(TIME_AND_BANDWIDTH));
    ret_val->duration = difftime(endTime, startTime);
    ret_val->bandwidth = bytes_received / ret_val->duration / (1024.0 * 1024.0); // Speed in MB/s
    ret_val->next = NULL;
    return ret_val;
}

int main(int argc, char *argv[])
{
    TIME_AND_BANDWIDTH *time_and_bandwidth_list = NULL, *last_time_and_bandwidth = NULL;
    char *buffer = (char *)malloc(FILE_SIZE);
    int port = -1;
    // char *algo = "";

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

    // int finishProgram = FALSE;
    // time_t startTime, endTime;
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
    TIME_AND_BANDWIDTH *current = time_and_bandwidth_list;
    // double sum_avarege = 0;
    // double sum_bandwidth = 0;
    int index = 0;
    while (current != NULL)
    {
        index++;
        printf("- Run #%d Data: Time=%.6fs; Speed=%.6fMB/s\n", index, current->duration, current->bandwidth);
        current = current->next;
    }
    // printf("got an exit command\n");
    /*
        while (!finishProgram)
        {
            // Create a buffer to store the received message.
            char buffer[BUFFER_SIZE] = {0};

            int bytes_received, total_bytes_received;
            time(&startTime); // Start time measurement
            while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0)
            {
                total_bytes_received += bytes_received;
                // Process received data (e.g., write to a file)
                // If the message receiving failed, print an error message and return 1.
                if (bytes_received < 0)
                {
                    perror("recv(2)");
                    close(client_sock);
                    close(sock);
                    return 1;
                }

                // If the amount of received bytes is 0, the client has disconnected.
                // Close the client's socket and continue to the next iteration.
                else if (bytes_received == 0)
                {
                    fprintf(stdout, "Client %s:%d disconnected\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    close(client_sock);
                    continue;
                }

                // Ensure that the buffer is null-terminated, no matter what message was received.
                // This is important to avoid SEGFAULTs when printing the buffer.
                if (buffer[BUFFER_SIZE - 1] != '\0')
                    buffer[BUFFER_SIZE - 1] = '\0';
            }
            time(&endTime); // End time measurement

            fprintf(stdout, "File transfer completed.\n");

            fprintf(stdout, "Waiting for Sender response...\n");
        }
    */
    return 1;
}
