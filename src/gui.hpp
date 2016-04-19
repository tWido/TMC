#ifndef GUI_HPP
#define	GUI_HPP

#include <qapplication.h>
#include <qpushbutton.h>
#include <QApplication>
#include <QWidget>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QPixmap>
#include <QIcon>

#include "device.hpp"

class GUI: public QWidget{
    public:
        QMainWindow *main;
        QMenuBar *menuBar;
        QMenu *control_menu;
        QMenu *device_menu;
        QMenu *help_menu;
        QLabel *dev_name;
        QLabel *dev_ser;
        QLabel *serial;
        QAction *device;
        QAction *device2;
        QRadioButton *chan_1;
        QRadioButton *chan_2;
        QRadioButton *chan_3;
        QPushButton *flash_button;
        QPushButton *home_button;
        QPushButton *forward;
        QPushButton *backward;
        QLabel *moves;
        QRadioButton *relm;
        QRadioButton *absm;
        QRadioButton *jogm;
        QRadioButton *velm;

        void Setup(){
            //main
            main = new QMainWindow(this);
            main->resize(800, 600);
            main ->setWindowTitle("Thorlabs motor controller");
            
            //Menu
            menuBar = new QMenuBar(main);
            menuBar->setGeometry(0, 0, 800, 40);
            menuBar->move(0,0);
            main->setMenuBar(menuBar);
            control_menu = new QMenu(menuBar);
            control_menu->setTitle("Control");
            device_menu = new QMenu(menuBar);
            device_menu->setTitle("Device");
            help_menu = new QMenu(menuBar);
            help_menu->setTitle("Help");
            menuBar->addMenu(control_menu);
            menuBar->addMenu(device_menu);
            menuBar->addMenu(help_menu);
            
            //Device labels
            dev_name = new QLabel(main);
            std::string dname = "Device: ";
            QFont font = dev_name->font();
            font.setPointSize(14);
            dev_name->setFont(font);
            dname.append(opened_device.dev_type_name);
            dev_name->setText(dname.c_str());
            dev_name->setGeometry(20,40,200,30);
            dev_ser = new QLabel(main);
            font = dev_ser->font();
            font.setPointSize(14);
            dev_ser->setFont(font);
            std::string serial = "SN: ";
            serial.append(opened_device.SN);
            dev_ser->setGeometry(20,65,200,30);
            dev_ser->setText(serial.c_str());
            
            //channel switch
            font.setPointSize(12);
            chan_1 = new QRadioButton(main);
            chan_1->setGeometry(30,200,80,50);
            chan_1->setText("Chan 1");
            chan_1->setChecked(true);
            chan_1->setFont(font);
            if (opened_device.channels == 2 || (opened_device.bays >=2 && opened_device.bay_used[1])){
                chan_2 = new QRadioButton(main);
                chan_2->setGeometry(120,200,80,50);
                chan_2->setText("Chan 2");
                chan_2->setFont(font);
                chan_1->setChecked(false);
            }
            if (opened_device.bays == 3 && opened_device.bay_used[2]){
                chan_3 = new QRadioButton(main);
                chan_3->setGeometry(210,200,80,50);
                chan_3->setText("Chan 3");
                chan_3->setFont(font);
                chan_1->setChecked(false);
            }    

            //flash and home buttons
            flash_button = new QPushButton(main);
            flash_button->setGeometry(50,300,100,50);
            flash_button->setText("Flash LED");
            home_button = new QPushButton(main);
            home_button->setGeometry(200,300,100,50);
            home_button->setText("Home");
            
            //up&down buttons
            forward = new QPushButton(main);
            forward->setIcon(QIcon("./src/triangle_up.png"));
            forward->setIconSize(QSize(65,65));
            forward->setGeometry(400,100,70,70);
            backward = new QPushButton(main);
            backward->setIcon(QIcon("./src/triangle_down.png"));
            backward->setIconSize(QSize(65,65));
            backward->setGeometry(400,180,70,70);
            
            //moves
            
            
            
            //status bar
            
            main->show();
        }
        
        
};

int run_gui(){
    char **argv = NULL;
    int argc = 0;
    QApplication a( argc, argv );
    GUI *gui = new GUI();
    gui->Setup();
    
    a.exec();
    return 0;
}


#endif	/* GUI_HPP */

