CC = g++

CFLAGS = -Wall -g -std=c++11

LIBS = -lpthread -lftd2xx

SRCS = ./src/main.cpp

MAIN = thorlabs_controller

BUILD_DIR = build

LINK_DIR=/usr/local/lib

all: $(SRCS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(SRCS) $(LIBS) $(CFLAGS) -o $(BUILD_DIR)/$(MAIN) -Wl,-rpath=$(LINK_DIR)
	mkdir $(BUILD_DIR)/restrictions

clean:
		rm $(BUILD_DIR)/$(MAIN)
