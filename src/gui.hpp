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
#include <QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QVBoxLayout>

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
        QLabel *moves_l;
        QRadioButton *relm;
        QRadioButton *absm;
        QRadioButton *jogm;
        QRadioButton *velm;
        QGroupBox *channels;
        QVBoxLayout *chan_box;
        QGroupBox *dir_moves;
        QVBoxLayout *ldir_moves;
        QGroupBox *moves;
        QGridLayout *lmoves;
        QLineEdit *reldist;
        QLineEdit *abspos;
        QLabel *reldl;
        QLabel *abspl;
        QPushButton *start;
        QPushButton *stop;
        
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
            dev_name->setGeometry(20,40,250,30);
            dev_ser = new QLabel(main);
            font = dev_ser->font();
            font.setPointSize(14);
            dev_ser->setFont(font);
            std::string serial = "SN: ";
            serial.append(opened_device.SN);
            dev_ser->setGeometry(20,65,250,30);
            dev_ser->setText(serial.c_str());
            
            //channel switch
            channels = new QGroupBox("Channels",main);
            chan_box = new QVBoxLayout();
            font.setPointSize(12);
            channels->setFont(font);
            chan_1 = new QRadioButton(main);
            chan_1->resize(100,50);
            chan_1->setText("Channel 1");
            chan_1->setChecked(true);
            chan_1->setFont(font);           
            chan_2 = new QRadioButton(main);
            chan_2->resize(100,50);
            chan_2->setText("Channel 2");
            chan_2->setFont(font);
            chan_2->setChecked(false);
            if (opened_device.channels == 2 || (opened_device.bays >=2 && opened_device.bay_used[1])){
                chan_2->setDisabled(false);
            }
            else chan_2->setDisabled(true);
            chan_3 = new QRadioButton(main);
            chan_3->resize(100,50);
            chan_3->setText("Channel 3");
            chan_3->setFont(font);
            chan_3->setChecked(false);
            if (opened_device.bays == 3 && opened_device.bay_used[2]){
                chan_3->setDisabled(false);
            }
            else chan_3->setDisabled(true);  
            chan_box->addWidget(chan_1);
            chan_box->addWidget(chan_2);
            chan_box->addWidget(chan_3);
            chan_box->setSpacing(10);
            channels->setLayout(chan_box);
            channels->setGeometry(40,120,200,200);

            //flash and home buttons
            flash_button = new QPushButton(main);
            flash_button->setGeometry(40,380,100,50);
            flash_button->setText("Flash LED");
            home_button = new QPushButton(main);
            home_button->setGeometry(180,380,100,50);
            home_button->setText("Home");
            
            //moves directional
            moves_l = new QLabel(main);
            moves_l->setText("Moves");
            moves_l->setGeometry(475,30,100,50);
            font.setPointSize(15);
            moves_l->setFont(font);
            forward = new QPushButton(main);
            forward->setIcon(QIcon("./src/triangle_up.png"));
            forward->setIconSize(QSize(65,65));
            forward->setGeometry(400,100,70,70);
            backward = new QPushButton(main);
            backward->setIcon(QIcon("./src/triangle_down.png"));
            backward->setIconSize(QSize(65,65));
            backward->setGeometry(400,180,70,70);
            
            dir_moves = new QGroupBox("Directional",main);
            ldir_moves = new QVBoxLayout();
            font.setPointSize(12);
            dir_moves->setFont(font);
            jogm = new QRadioButton("Jog",main);
            jogm->resize(100,50);
            velm = new QRadioButton("Velocity",main);
            velm->resize(100,50);
            ldir_moves->addWidget(jogm);
            ldir_moves->addWidget(velm);
            ldir_moves->setSpacing(8);
            dir_moves->setLayout(ldir_moves);
            dir_moves->setGeometry(500,80,150,175);
            
            //moves non-directional
            moves = new QGroupBox(main);
            lmoves = new QGridLayout();
            absm = new QRadioButton("Absolute",main);
            absm->resize(100, 50);
            abspl = new QLabel("Position",main);
            abspl->resize(50,50);
            abspos = new QLineEdit(main);
            abspos->setInputMask("999999999");
            abspos->setText("0");
            abspos->resize(80,25);
            reldist = new QLineEdit(main);
            reldist->setText("0");
            reldist->setInputMask("999999999");
            reldist->resize(80,25);
            relm = new QRadioButton("Relative",main);
            relm->resize(100, 50);
            reldl = new QLabel("Distance",main);
            reldl->resize(50,50);
            start = new QPushButton("Start",main);
            start->resize(40,25);
            lmoves->setVerticalSpacing(4);
            lmoves->addWidget(absm,0,0);
            lmoves->addWidget(abspos,0,2);
            lmoves->addWidget(abspl,0,1);
            lmoves->addWidget(relm,1,0);
            lmoves->addWidget(reldist,1,2);
            lmoves->addWidget(reldl,1,1);
            lmoves->addWidget(start);
            moves->setLayout(lmoves);
            moves->setGeometry(400,250,300,200);
            
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

