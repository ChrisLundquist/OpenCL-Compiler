
CC            = gcc
CXX           = g++
DEFINES       = 
CFLAGS        = -pipe -O2 -Wall -Wextra -pedantic $(DEFINES) -c
CXXFLAGS      = -pipe -O2 -Wall -Wextra -pedantic $(DEFINES) -c
INCPATH       = -I ./include
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS) -lOpenCL
EXE           =

all: compiler$(EXE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) $(LIBS) $<

compiler$(EXE): compiler.o platform.o device.o
	    $(CXX) $(LFLAGS) $(LIBS) -o $@ $+

clean:
	rm *.o compiler$(EXE)

test:
	./compiler$(EXE) samples/test.cl
