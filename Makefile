
CXXFLAGS += -O3
CXXFLAGS += -Wall
CXXFLAGS += -std=c++11
CXXFLAGS += -DNDEBUG

CXXFLAGS += $(CXXEXTRA)

hex: hex.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f hex
	rm -f *.o
