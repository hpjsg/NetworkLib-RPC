where-am-i = $(abspath $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
ROOT = $(dir $(call where-am-i))
CC = g++ 
CXXFLAGS = -std=c++11 -g  -I $(ROOT) -pthread
LDFLAGS = -lpthread -lprotobuf -lz -lhiredis -lssl -lcrypto
SOURCE   = $(wildcard base/*.cpp net/*.cpp rpc/*.cpp)
MAINSOURCE = $(wildcard base/*.cpp net/*.cpp)
BINARIES = client_test server_test
TARGET = rpc_client_test rpc_server_test
OBJS    = $(patsubst %.cpp,%.o,$(SOURCE))

.PHONY:clean all

all: $(BINARIES) $(TARGET)

$(BINARIES):
	$(CC) $(CXXFLAGS) -o $@ $(MAINSOURCE) $(filter %.cpp,$^) $(LDFLAGS) 

$(TARGET):
	$(CC) $(CXXFLAGS) -o $@ $(SOURCE) $(filter %.cpp,$^) $(LDFLAGS) 


debug:
	@echo $(SOURCE)
	@echo $(PBFLAGS)

clean:
	rm -f $(BINARIES) $(TARGET) core

client_test: test/client_test.cpp
server_test: test/server_test.cpp
rpc_client_test: test/rpc_client_test.cpp
rpc_server_test: test/rpc_server_test.cpp




