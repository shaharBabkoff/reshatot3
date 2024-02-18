CC=gcc
CFLAGS=-Wall -g

# Targets
RECEIVER_TARGET=TCP_Receiver  # Output executable named TCP_Receiver.c
SENDER_TARGET=TCP_Sender        # Assuming you want a standard name for the sender executable

all: $(RECEIVER_TARGET) $(SENDER_TARGET)

$(RECEIVER_TARGET): TCP_Receiver.c
	$(CC) $(CFLAGS) -o $(RECEIVER_TARGET) TCP_Receiver.c

$(SENDER_TARGET): TCP_Sender.c
	$(CC) $(CFLAGS) -o $(SENDER_TARGET) TCP_Sender.c

clean:
	rm -f $(RECEIVER_TARGET) $(SENDER_TARGET)