CXX ?= g++
CFLAGS=-I../../../lmic
LDFLAGS=-lwiringPi

ttn-otaa-send: ttn-otaa-send.cpp
	cd ../../../lmic && $(MAKE)
	$(CXX) $(CFLAGS) -o ttn-otaa-send ttn-otaa-send.cpp ../../../lmic/*.o $(LDFLAGS)

all: ttn-otaa-send

run: ./ttn-otaa-send

.PHONY: clean

clean:
	rm -f *.o ttn-otaa-send
