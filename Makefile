TARGETS = netstore-client

CC     = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LFLAGS = -Wall

all: $(TARGETS)

netstore-client: netstore-client.o protocol.o countdown.o children.o found_files.o servers_with_capacity.o client_upload_utils.o client_utils.o client_download_utils.o input_parsers.o client_commands.o
	$(CC) $(LFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *~ *.bak
