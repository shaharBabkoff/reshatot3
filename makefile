# Compiler to use
CC=gcc

# Compiler flags
CFLAGS=-Wall -I./include

# Executable names
RECEIVER=TCP_Receiver
SENDER=TCP_Sender
RUDP_RECEIVER=RUDP_Receiver
RUDP_SENDER=RUDP_Sender

# Compile RUDP_API.c into an object file
RUDP_API_OBJ=RUDP_API.o

# Header files
HEADERS=RUDP_API.h

all: $(RECEIVER) $(SENDER) $(RUDP_RECEIVER) $(RUDP_SENDER)

$(RECEIVER): TCP_Receiver.c 
	$(CC) $(CFLAGS) -o $(RECEIVER) TCP_Receiver.c

$(SENDER): TCP_Sender.c 
	$(CC) $(CFLAGS) -o $(SENDER) TCP_Sender.c

$(RUDP_RECEIVER): RUDP_Receiver.c $(RUDP_API_OBJ)
	$(CC) $(CFLAGS) -o $(RUDP_RECEIVER) RUDP_Receiver.c $(RUDP_API_OBJ)

$(RUDP_SENDER): RUDP_Sender.c $(RUDP_API_OBJ)
	$(CC) $(CFLAGS) -o $(RUDP_SENDER) RUDP_Sender.c $(RUDP_API_OBJ)

$(RUDP_API_OBJ): RUDP_API.c $(HEADERS)
	$(CC) $(CFLAGS) -c RUDP_API.c

clean:
	rm -f $(RECEIVER) $(SENDER) $(RUDP_RECEIVER) $(RUDP_SENDER) $(RUDP_API_OBJ)
