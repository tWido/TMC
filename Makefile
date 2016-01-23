CC = g++

CFLAGS = -Wextra -Wall

LIBS = -lpthread -lftd2xx

SRCS = ./src/main.cpp

MAIN = thorlabs_controller

BUILD_DIR = build

all: $(SRCS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(SRCS) $(LIBS) $(CFLAGS) -o $(BUILD_DIR)/$(MAIN)

clean:
		rm $(BUILD_DIR)/$(MAIN)
