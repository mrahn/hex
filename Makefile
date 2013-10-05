
CFLAGS += -O3
CFLAGS += -Wall
CFLAGS += -std=c99
CFLAGS += $(CXXEXTRA)

###########################################################################

hex: hex.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

###########################################################################

.PHONY: clean

clean:
	rm -f hex
	rm -f *.o
