CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=exporter.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LIBS=-lpcap -lrt
EXECUTABLE=isa_exporter

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ ${LIBS}

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
    
