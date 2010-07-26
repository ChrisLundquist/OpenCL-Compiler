
CC            = gcc
CXX           = g++
DEFINES       = 
#DEBUG_FLAGS define this from the command line
CFLAGS        = -pipe -O2 -Wall -Wextra -pedantic $(DEFINES) -c
CXXFLAGS      = -pipe -O2 -Wall -Wextra -pedantic $(DEFINES) -c
INCPATH       = -I ./include
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS) -lOpenCL
EXE           =

all: clc$(EXE)

%.o: %.cpp
	$(CXX) $(DEBUG_FLAGS) $(CXXFLAGS) $(INCPATH) $(LIBS) $<

clc$(EXE): clc.o platform.o device.o
	    $(CXX) $(DEBUG_FLAGS) $(LFLAGS) $(LIBS) -o $@ $+

clean:
	rm *.o clc$(EXE)

test: clc$(EXE)
	./clc$(EXE) samples/test.cl

install:
	install clc$(EXE) /usr/local/bin 
