#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "RUDP_API.h"
#include "RUDP.h"

// #define RECEIVER_IP "127.0.0.1"  // IP address of the receiver
// #define SERVER_PORT 5061         // Port number the receiver is listening on
// #define BUFFER_SIZE 1024 // Size of the buffer for sending data
#define FILE_SIZE 3532695
typedef struct data
{
    size_t size;
    char data[MAX_DATA_SIZE];
} data;

void usage()
{
    printf("Usage: ./RUDP_Sender -ip IP -p PORT\n");
    printf("PORT - The RUDP port of the Sender.\n");
}
void transmit_file_data(RUDP_Socket *sock, struct data *data, int arraySize)
{
    rudp_send(sock, NULL, 0, BEGIN_FILE_TRANSMITION);
    for (int i = 0; i < arraySize; i++)
    {
        rudp_send(sock, data[i].data, data[i].size, 0);
    }
    rudp_send(sock, NULL, 0, END_FILE_TRANSMITION);
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
    ip = argv[2];
    int array_size = FILE_SIZE / MAX_DATA_SIZE;
    if (FILE_SIZE % MAX_DATA_SIZE != 0)
    {
        array_size++;
    }
    struct data *data_array = (struct data *)malloc(array_size * sizeof(struct data));

    // char *buffer = (char *)malloc(FILE_SIZE);
    FILE *fp;
    char *filename = "testFile";
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error in reading file.");
        exit(1);
    }
    for (int i = 0; i < array_size; i++)
    {
        int bytes_read = fread(data_array[i].data, 1, MAX_DATA_SIZE, fp);
        if (ferror(fp))
        {
            perror("fread(2)");
            exit(1);
        }
        else
        {
            data_array[i].size = bytes_read;
        }
    }
    printf("array size is %d, last buffer size is %d\n", array_size, (int)data_array[array_size - 1].size);
    // for (int i = 0; i < MAX_DATA_SIZE; i++)
    // {
    //     printf("%c", data_array[3][i]);
    // }

    RUDP_Socket *sock = rudp_socket(0, 0);
    if (rudp_connect(sock, ip, port) == 1)
    {
        printf("Successfully connected to the receiver !\n");

        char userChoise = '1';

        while (userChoise == '1')
        {

            transmit_file_data(sock, data_array, array_size);
            printf("please enter your choice: (1 for resending, any other character to quit)\n");
            scanf(" %c", &userChoise);
            printf("user choice %c\n", userChoise);
        }
        rudp_disconnect(sock);
    } else {
        printf("failed to connect to the receiver, please make sure the receiver is up and running on %s:%d and try again\n", ip, port);
    }
    rudp_close(sock);
    free(data_array);
    return 0;
}
