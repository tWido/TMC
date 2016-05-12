CXX = g++

CFLAGS = -Wall -Wextra -g -std=c++11 -fPIE -O2 -pipe

LIBS = -lpthread -lftd2xx

LFLAGS = -L/home/wido/Git/bc-thorlabs/drivers/release/build

MAIN = thorlabs_controller

BUILD_DIR = bin

DEFINE_GUI = -D _GUI_

QTLIBS= -lQt5Widgets -lQt5Gui -lQt5Core -lGL

QTINCLUDES= -I/usr/include/qt5 -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtCore 


all: $(BUILD_DIR)/api.o $(BUILD_DIR)/main.o
	mkdir -p $(BUILD_DIR)/restrictions
	$(CXX) $(BUILD_DIR)/api.o $(BUILD_DIR)/main.o $(CFLAGS) $(LIBS) $(LFLAGS) -o $(BUILD_DIR)/$(MAIN) -Wl,-rpath=./drivers/release/build
	
gui: 	$(BUILD_DIR)/api.o $(BUILD_DIR)/gmain.o $(BUILD_DIR)/moc_gui.o $(BUILD_DIR)/gui.o
	mkdir -p $(BUILD_DIR)/restrictions
	$(CXX) $(BUILD_DIR)/api.o $(BUILD_DIR)/gmain.o $(BUILD_DIR)/moc_gui.o $(BUILD_DIR)/gui.o $(CFLAGS) $(LIBS) $(LFLAGS)  $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/$(MAIN) -Wl,-rpath=./drivers/release/build 
	
clean:
	rm bin/*.o
	
uninstall: 
	rm -rf $(BUILD_DIR) 

$(BUILD_DIR)/api.o: ./src/api.cc ./src/api.hpp ./src/messages.hpp ./src/message_codes.hpp ./src/device.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) ./src/api.cc -c  $(CFLAGS) $(LIBS) $(LFLAGS)  -o $(BUILD_DIR)/api.o
	
$(BUILD_DIR)/moc_gui.o: ./src/moc_gui.cpp
	$(CXX) ./src/moc_gui.cpp -c  $(CFLAGS) $(LIBS) $(LFLAGS) $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/moc_gui.o
	
$(BUILD_DIR)/gui.o: ./src/gui.cpp ./src/gui.hpp
	$(CXX) ./src/gui.cpp -c  $(CFLAGS) $(LIBS) $(LFLAGS) $(QTLIBS) $(QTINCLUDES) -o $(BUILD_DIR)/gui.o
	
$(BUILD_DIR)/main.o: ./src/main.cpp ./src/init.hpp ./src/cmd.hpp
	$(CXX) ./src/main.cpp -c $(CFLAGS) $(LIBS) $(LFLAGS) -o $(BUILD_DIR)/main.o
	
$(BUILD_DIR)/gmain.o: ./src/main.cpp ./src/init.hpp ./src/cmd.hpp
	$(CXX) ./src/main.cpp -c  $(CFLAGS) $(LIBS) $(LFLAGS) $(QTLIBS) $(QTINCLUDES) $(DEFINE_GUI) -o $(BUILD_DIR)/gmain.o 