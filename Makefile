
JUDY_HOME = $(HOME)

ifndef SIZE
  SIZE = 2
endif

CFLAGS += -O3
CFLAGS += -Wall
CFLAGS += -std=c99
CFLAGS += -I$(JUDY_HOME)/include
CFLAGS += -DSIZE=$(SIZE)

LDFLAGS += -L$(JUDY_HOME)/lib
LDFLAGS += -Wl,--rpath -Wl,$(JUDY_HOME)/lib

LIB += Judy

###########################################################################

hex: hex.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(addprefix -l,$(LIB)) -o $@

###########################################################################

.PHONY: clean

clean:
	rm -f hex
	rm -f *.o
