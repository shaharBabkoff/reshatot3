#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h> // For gettimeofday()
#include "RUDP_API.h"
#include "RUDP.h"

/*
 * @brief The maximum number of clients that the server can handle.
 * @note The default maximum number of clients is 1.
 */
#define MAX_CLIENTS 1

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
 */
// #define BUFFER_SIZE 1024
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

TIME_AND_BANDWIDTH *receive_file(RUDP_Socket *sock)
{
    TIME_AND_BANDWIDTH *ret_val = NULL;
    struct timeval startTime, endTime;
    int bytes_received = 0;
    gettimeofday(&startTime, NULL);
    while (true)
    {
        char buffer[MAX_DATA_SIZE];
        unsigned char udf = 0;
        int res = rudp_recv(sock, buffer, sizeof(buffer), &udf);
        if (udf == END_FILE_TRANSMITION)
        {
            gettimeofday(&endTime, NULL);
            printf("bytes received: %d\n", bytes_received);
            ret_val = (TIME_AND_BANDWIDTH *)malloc(sizeof(TIME_AND_BANDWIDTH));
            ret_val->duration = ((endTime.tv_sec - startTime.tv_sec) * 1000.0) + ((endTime.tv_usec - startTime.tv_usec) / 1000.0); // Convert to milliseconds
            ret_val->bandwidth = (bytes_received / ret_val->duration / (1024.0 * 1024.0)) * 1000;                                           // Speed in MB/s
            ret_val->next = NULL;
            break;
        }
        else if (res <= 0)
        {
            break;
        } else {
            bytes_received += res;
        }
    }
    printf("receive_file received %d byted\n", bytes_received);
    return ret_val;
}

int main(int argc, char *argv[])
{
    TIME_AND_BANDWIDTH *time_and_bandwidth_list = NULL, *last_time_and_bandwidth = NULL;
    // char *buffer = (char *)malloc(FILE_SIZE);
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

   

    RUDP_Socket *sock = rudp_socket(1, port);
    if (rudp_accept(sock) == 0)
    {
        exit(-1);
    }
    while (true)
    {
        unsigned char udf = 0;
        rudp_recv(sock, NULL, 0, &udf);
        if (udf == BEGIN_FILE_TRANSMITION)
        {

            TIME_AND_BANDWIDTH *tmp = NULL;
            if ((tmp = receive_file(sock)) == NULL)
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
        else
        {
            break;
        }
    }
    rudp_disconnect(sock);
    rudp_close(sock);
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
    return 0;
}
