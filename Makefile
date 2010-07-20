
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

all: compiler$(EXE)

%.o: %.cpp
	$(CXX) $(DEBUG_FLAGS) $(CXXFLAGS) $(INCPATH) $(LIBS) $<

compiler$(EXE): compiler.o platform.o device.o
	    $(CXX) $(DEBUG_FLAGS) $(LFLAGS) $(LIBS) -o $@ $+

clean:
	rm *.o compiler$(EXE)

test: compiler$(EXE)
	./compiler$(EXE) samples/test.cl
