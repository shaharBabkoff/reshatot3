#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>


#define Receiver_PORT 5060
#define MAX_senderS 1
#define BUFFER_SIZE 1024
#define EXIT_TERMINAL "EXIT"


int main(void){
 // The variable to store the socket file descriptor.
   // int sock = -1;

    // The variable to store the Receiver's address.
    struct sockaddr_in Receiver;

    // The variable to store the sender's address.
    struct sockaddr_in sender;

    // Stores the sender's structure length.
    socklen_t sender_len = sizeof(sender);

    // Create a message to send to the sender.
    // char *message = "hello from receiver\n";

    // // Get the message length.
    // int messageLen = strlen(message) + 1;

    // The variable to store the socket option for reusing the receiver's address.
    int opt = 1;

    time_t start,end;
    int listenfd;
    
    ssize_t total_bytes_received = 0;

    // Reset the Receiver and sender structures to zeros.
    memset(&Receiver, 0, sizeof(Receiver));
    memset(&sender, 0, sizeof(sender));

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

     if (listenfd == -1)
    {
        perror("socket(2)");
        return 1;
    }
     // Set the socket option to reuse the Receiver's address.
    // This is useful to avoid the "Address already in use" error message when restarting the Receiver.
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(2)");
        close(listenfd);
        return 1;
    }

    // Set the Receiver's address to "0.0.0.0" (all IP addresses on the local machine).
    Receiver.sin_addr.s_addr = INADDR_ANY;

    // Set the Receiver's address family to AF_INET (IPv4).
    Receiver.sin_family = AF_INET;

    // Set the Receiver's port to the specified port. Note that the port must be in network byte order.
    Receiver.sin_port = htons(Receiver_PORT);

     // Try to bind the socket to the Receiver's address and port.
    if (bind(listenfd, (struct sockaddr *)&Receiver, sizeof(Receiver)) < 0)
    {
        perror("bind(2)");
        close(listenfd);
        return 1;
    }

    // Try to listen for incoming connections.
    if (listen(listenfd, MAX_senderS) < 0)
    {
        perror("listen(2)");
        close(listenfd);
        return 1;
    }

fprintf(stdout, "Listening for incoming connections on port %d...\n", Receiver_PORT);
 while (1)
    {
        // Try to accept a new sender connection.
        int connfd = accept(listenfd, (struct sockaddr *)&sender, &sender_len);

        // If the accept call failed, print an error message and return 1.
        if (connfd < 0)
        {
            perror("accept(2)");
            close(listenfd);
            return 1;
        }

        // Print a message to the standard output to indicate that a new sender has connected.
        fprintf(stdout, "sender %s:%d connected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));

        // Create a buffer to store the received message.
        char buffer[BUFFER_SIZE] = {0};

        // Receive the file
    int bytes_received;
    time(&start); // Start time measurement
    while ((bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0) {
        total_bytes_received += bytes_received;
        // Process received data (e.g., write to a file)
    }
   time(&end); // End time measurement

    printf("File transfer completed.\n");
    printf("Waiting for Sender response...\n");

    // Here, add logic to wait for and process the sender's response, such as an exit message

    printf("Sender sent exit message.\n");

    // Calculate and print statistics
    double transfer_time = difftime(end, start);
    double speed = total_bytes_received / transfer_time / (1024.0 * 1024.0); // Speed in MB/s

    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    printf("- Run #1 Data: Time=%.2fs; Speed=%.2fMB/s\n", transfer_time, speed);
    printf("-\n");
    printf("- Average time: %.2fs\n", transfer_time); // Assuming a single run for simplicity
    printf("- Average bandwidth: %.2fMB/s\n", speed);
    printf("----------------------------------\n");

    printf("Receiver end.\n");

    // Cleanup
    close(connfd);
    close(listenfd);

    return 0;
}
}