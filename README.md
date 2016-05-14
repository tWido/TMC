Thorlabs motor controller 
==============================

Program for controlling Thorlabs stepper motors
Version: 1.0

## Instalation
* Unpack drivers. Bash script unpack_driver.sh should determine which drivers to use. If not, unpack manually in drivers project directory.
* Use "make" for only cmd application and "make gui" for application with both cmd and graphical interface. (See requirements fo graphical interface)

## Requirements
* Any Linux OS (tested on Ubuntu and Arch)
* Linux kernel 2.4 and higher
* STD c++11
* Qt library version 4 an higher if using graphical interface

Note: Accessing USB device may require priviledges. On systems using systemd add new rule to "/etc/udec/rules.d" directory. 
Example rule, that allows device use for users in group device_group : SUBSYSTEM=="usb", ATTRS{idVendor}=="1234", ATTRS{idProduct}=="5678", ACTION=="add", GROUP="device_group", MODE="0664"

## Supported controller devices
* TDC001
* MST601 / MST602
* all BBD devices
* all BSC devices
* OST001
* ODC001
* TST001
* TBD001

Feel free to report any bugs or submit ideas.
Github accout: tWido
Email: tomas.wido (at) gmail.com