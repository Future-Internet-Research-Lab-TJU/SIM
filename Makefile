EXEC += sim
all: $(EXEC)

CFLAGS = -Wall -std=c++11 -O3
HEADER += hash.h datatypes.hpp util.h adaptor.hpp RrConfig.h SIM_heavy.hpp SIM_light.hpp SIM.hpp
SRC += hash.c adaptor.cpp RrConfig.cpp
LIBS= -lpcap 

sim: main.cpp $(SRC) $(HEADER) 
	g++ $(CFLAGS) $(INCLUDES) -o $@ $< $(SRC) $(LIBS)

clean:
	rm -rf $(EXEC)
	rm -rf *log*
	rm -rf *out*
