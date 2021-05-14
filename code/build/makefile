#LDFLAGS += -lm -lpthread -lgcc_s

CFLAGS += -I./ -I./paho/ -I./sqlite -I./cJSON
CXXFLAGS += -I./ -I./paho/ -I./sqlite -I./cJSON
#CFLAGS += -I../socketmsg/
CFLAGS += -g -fPIC -o2 -std=c11 -DOPENSSL
CXXFLAGS += -g -fPIC -o2 -std=c++11 -DOPENSSL
LDFLAGS += -L./openssl -L../nvr/lib/IMX6ULL/libevent -L../nvr/lib/IMX6ULL/paho -L../nvr/lib/IMX6ULL/sqlite -levent -levent_core -levent_extra -levent_openssl -levent_pthreads -lsqlite3 -lpthread  -lpaho-mqtt3cs 
#LDFLAGS += -L./ -levent -levent_core -levent_extra -levent_openssl -levent_pthreads -lsqlite3 -lpthread -lpaho-mqtt3a -lpaho-mqtt3as #-lpaho-mqtt3c

#CC = gcc
CC = arm-linux-gnueabihf-gcc
CXX = arm-linux-gnueabihf-g++
AR = arm-linux-gnueabihf-ar

SRCS=$(wildcard *.cpp *.c cjson/*.c)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))
#OBJS1 = server.o \

#OBJS2 = client.o \

TARGET = force_server
#TARGET1 = client

all:  $(TARGET) #$(TARGET1)	
	
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	#$(STRIP) $(TARGET)

#$(TARGET1): $(OBJS2)
#	$(CC) -o $@ $^ $(LDFLAGS)

	
.c.cpp.o:
	$(CXX) -c $(CXXFLAGS) $^ -o $@ 
clean:
	$(RM) $(TARGET) $(TARGET1) $(OBJS)#$(OBJS1) $(OBJS2)
