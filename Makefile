
JUDY_HOME = $(HOME)

CXXFLAGS += -O3
CXXFLAGS += -Wall
CXXFLAGS += -std=c++11
CXXFLAGS += -DNDEBUG
CXXFLAGS += -I$(JUDY_HOME)/include
CXXFLAGS += $(CXXEXTRA)

LDLFAGS += -L$(JUDY_HOME)/lib

LIB += Judy

###########################################################################

hex: hex.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< $(addprefix -l,$(LIB)) -o $@

###########################################################################

.PHONY: clean

clean:
	rm -f hex
	rm -f *.o
