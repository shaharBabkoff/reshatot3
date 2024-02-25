# Compiler to use
CC=gcc

# Compiler flags
CFLAGS=-Wall

# Executable names
RECEIVER=TCP_Receiver_New
SENDER=TCP_Sender

all: $(RECEIVER) $(SENDER)

$(RECEIVER): TCP_Receiver_New.c
	$(CC) $(CFLAGS) -o $(RECEIVER) TCP_Receiver_New.c

$(SENDER): TCP_Sender.c
	$(CC) $(CFLAGS) -o $(SENDER) TCP_Sender.c

clean:
	rm -f $(RECEIVER) $(SENDER)
