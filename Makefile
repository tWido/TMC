CXX = g++

CFLAGS = -Wall -Wextra -g -std=c++11 -fPIE -O2 -pipe

LIBS = -lpthread -lpthread -L/home/wido/Git/bc-thorlabs/drivers/release/build -lftd2xx

SRCS = ./src/gui.cpp ./src/moc_gui.cpp  ./src/main.cpp  ./src/api.cc

MAIN = thorlabs_controller

BUILD_DIR = bin

DEFINES = 

QTLIBS= -lQt5Widgets -lQt5Gui -lQt5Core -lGL

QTINCLUDES= -I/usr/include/qt5 -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtCore 

all: $(BUILD_DIR)/api.o $(BUILD_DIR)/main.o $(BUILD_DIR)/moc_gui.o $(BUILD_DIR)/gui.o
	mkdir -p $(BUILD_DIR)/restrictions
	$(CXX) $(BUILD_DIR)/api.o $(BUILD_DIR)/main.o $(BUILD_DIR)/moc_gui.o $(BUILD_DIR)/gui.o $(LIBS) $(CFLAGS) $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/$(MAIN) -Wl,-rpath=./drivers/release/build
	
clean:
	rm bin/*.o
	
uninstall: 
	rm -rf $(BUILD_DIR) 

$(BUILD_DIR)/api.o: ./src/api.cc ./src/api.hpp ./src/messages.hpp ./src/message_codes.hpp ./src/device.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) ./src/api.cc -c $(LIBS) $(CFLAGS)  -o $(BUILD_DIR)/api.o
	
$(BUILD_DIR)/moc_gui.o: ./src/moc_gui.cpp
	$(CXX) ./src/moc_gui.cpp -c $(LIBS) $(CFLAGS) $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/moc_gui.o
	
$(BUILD_DIR)/gui.o: ./src/gui.cpp ./src/gui.hpp
	$(CXX) ./src/gui.cpp -c $(LIBS) $(CFLAGS) $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/gui.o
	
$(BUILD_DIR)/main.o: ./src/main.cpp ./src/init.hpp ./src/cmd.hpp
	$(CXX) ./src/main.cpp -c $(LIBS) $(CFLAGS) $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/main.o