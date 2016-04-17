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

class GUI: public QWidget{
    public:
        QMainWindow *main;
        QMenuBar *menuBar;
        QMenu *control_menu;
        QMenu *device_menu;
        QMenu *help_menu;
        QLabel *dev_name;
        QLabel *serial;
        QAction *device;
        QAction *device2;

        void Setup(){
            main = new QMainWindow(this);
            main->resize(800, 600);
            main ->setWindowTitle("Thorlabs motor controller");
            dev_name = new QLabel(main);
            dev_name->setText("waaat");
            dev_name->setGeometry(50,150,100,20);
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

