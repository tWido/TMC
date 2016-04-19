CC = g++

CFLAGS = -Wall -g -std=c++11 

LIBS = -lpthread -lftd2xx -lpthread

SRCS = ./src/main.cpp

MAIN = thorlabs_controller

BUILD_DIR = build

LINK_DIR=/usr/local/lib

QTFLAGS= -lQt5Widgets -lQt5Gui -lQt5Core

QTINCLUDES= -I/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++-64 -I/usr/include/qt5 -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtCore

all: $(SRCS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(SRCS) $(LIBS) $(CFLAGS) -o $(BUILD_DIR)/$(MAIN) -Wl,-rpath=$(LINK_DIR) -fPIE -L/usr/X11R6/lib64  -L/usr/lib/x86_64-linux-gnu -lGL $(QTFLAGS) $(QTINCLUDES)
	mkdir -p $(BUILD_DIR)/restrictions

clean:
		rm $(BUILD_DIR)/$(MAIN)
