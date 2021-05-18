LDFLAGS += -lm -lpthread -lgcc_s 
$(warning  $(TARGET_PLATFORM))
PROJECTROOT := ${shell pwd}/../..
ifeq ($(TARGET_PLATFORM), imx6ull)
LDFLAGS += -L$(PROJECTROOT)/lib/IMX6ULL/libevent -levent
LDFLAGS += -L$(PROJECTROOT)/lib/IMX6ULL/sqlite -lsqlite3
LDFLAGS += -L$(PROJECTROOT)/lib/IMX6ULL/openssl -lcrypto -lssl
else
ifeq ($(TARGET_PLATFORM), x86)
LDFLAGS += -L$(PROJECTROOT)/lib/x86 -lmodbus
LDFLAGS += -L$(PROJECTROOT)/lib/x86 -lsqlite3
CC = gcc
CXX = g++
AR = ar
endif
endif



CFLAGS += -I$(PROJECTROOT)/code/include/libmodbus/
CFLAGS += -I$(PROJECTROOT)/code/include/modbus_handler
CFLAGS += -I$(PROJECTROOT)/code/include/sqlite
CFLAGS += -I$(PROJECTROOT)/code/include/sqlite_helper
CFLAGS += -I$(PROJECTROOT)/code/include/
CXXFLAGS += -I$(PROJECTROOT)/code/include/libmodbus/
CXXFLAGS += -I$(PROJECTROOT)/code/include/modbus_handler
CXXFLAGS += -I$(PROJECTROOT)/code/include/sqlite
CXXFLAGS += -I$(PROJECTROOT)/code/include/sqlite_helper
CXXFLAGS += -I$(PROJECTROOT)/code/include/
#CFLAGS += -I../socketmsg/
CFLAGS += -g -fPIC -o2 -std=c11
CXXFLAGS += -g -fPIC -o2 -std=c++11
#LDFLAGS += -L./ -levent -levent_core -levent_extra -levent_openssl -levent_pthreads -lsqlite3 -lpthread -lpaho-mqtt3a -lpaho-mqtt3as #-lpaho-mqtt3c
$(warning  $(LDFLAGS))
#AR = arm-linux-gnueabihf-ar

SRCS=$(wildcard $(PROJECTROOT)/code/src/modbus_handler/*.cpp $(PROJECTROOT)/code/src/modbus_handler/*.c $(PROJECTROOT)/code/src/sqlite_helper/*.cpp $(PROJECTROOT)/code/src/sqlite_helper/*.c $(PROJECTROOT)/code/src/*.c $(PROJECTROOT)/code/src/*.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))
#OBJS1 = server.o \

#OBJS2 = client.o \

TARGET = framework
#TARGET1 = client

all:  $(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	#$(STRIP) $(TARGET)

#$(TARGET1): $(OBJS2)
#	$(CC) -o $@ $^ $(LDFLAGS)

	
.c.cpp.o:
	$(CXX) -c $(CXXFLAGS) $^ -o $@ 
clean:
	$(RM) $(TARGET) $(OBJS)#$(OBJS1) $(OBJS2)
