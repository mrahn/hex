
JUDY_HOME = $(HOME)

CFLAGS += -O3
CFLAGS += -Wall
CFLAGS += -std=c99
CFLAGS += -I$(JUDY_HOME)/include
CFLAGS += $(CXXEXTRA)

LDFLAGS += -L$(JUDY_HOME)/lib

LIB += Judy

###########################################################################

hex: hex.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(addprefix -l,$(LIB)) -o $@

###########################################################################

.PHONY: clean

clean:
	rm -f hex
	rm -f *.o
