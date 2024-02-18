# include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define Receiver_PORT 5060
#define Receiver_IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024
#define EXIT_TERMINAL "EXIT"
 
 int main(void){
    // The variable to store the socket file descriptor
    int sock=-1;
    // The variable to store the Receiver's address.
    struct sockaddr_in Sender;
    // Create a message to send to the Receiver.
    char *message = "Hello from sender";
    // Create a buffer to store the received message.
    char buffer[BUFFER_SIZE] = {0};
    FILE *file;
    // Reset the Receiver structure to zeros.
    memset(&Sender, 0, sizeof(Sender));
    // Try to create a TCP socket 
    fprintf(stdout,"try to create TCP");
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
      {
        perror("socket(2)");
        return 1;
      }

 if (inet_pton(AF_INET, Receiver_IP_ADDRESS, &Sender.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }
// Set the Receiver's address family to AF_INET (IPv4).
    Sender.sin_family = AF_INET;

    // Set the Receiver's port to the defined port. Note that the port must be in network byte order,
    // so we first convert it to network byte order using the htons function.
    Sender.sin_port = htons(Receiver_PORT);
    
    fprintf(stdout, "Connecting to %s:%d...\n", Receiver_IP_ADDRESS, Receiver_PORT);

    // Try to connect to the Receiver using the socket and the Receiver structure.
    if (connect(sock, (struct sockaddr *)&Sender, sizeof(Sender)) < 0)
    {
        perror("connect(2)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Successfully connected to the receiver !\n"
    "Sending message to the Receiver: %s\n", message);
// open file
file =fopen("check","rb");
if (file == NULL){
    perror("file open failed");
    exit (EXIT_FAILURE);
}

do {
    fseek(file, 0, SEEK_SET);

    // read and send file
    int byte_read;
    while ((byte_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(sock, buffer, byte_read, 0);
    }
    
    // Always close the file here, after it's done being used
    fclose(file);
    char user_input[10];
    printf("Send the file again? (yes/no): ");
    scanf("%s", user_input);
    
    if (strcmp(user_input, "no") == 0) {
        send(sock, EXIT_TERMINAL, strlen(EXIT_TERMINAL), 0);
        break;
    } else {
        // Reopen the file for the next iteration
        file = fopen("check", "rb");
        if (file == NULL) {
            perror("file open failed");
            exit(EXIT_FAILURE);
        }
    }

} while (1);
    // Close the connection
    close(sock);
    fprintf(stdout, "Connection closed!\n");
    return 0;
 }