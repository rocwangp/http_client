CC := g++

INCLUDE := -I./

LIBS := -lpthread -fno-stack-protector

DDEBUG := 

CXXFLAGS := -g -Wall -D_REENTRANT $(DDEBUG)

OBJECTS := Socket.o http_client.o client.o http_request.o http_response.o

TARGET := http_client

.cpp.o:
	$(CC) $(CXXFLAGS) $(INCLUDE) -c $<

$(TARGET) : $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LIBS)

.PHONY : clean
clean:
	-rm -f $(TARGET) $(OBJECTS)
