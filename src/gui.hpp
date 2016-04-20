#ifndef GUI_HPP
#define	GUI_HPP

#include <string> 
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
#include <QPalette>

#include "device.hpp"

class GUI: public QMainWindow{
    
    public slots:
        void openDevOptions(){};
        void openMoveOptions(){};
        void openHelp(){};
        void openDoc(){};
        void switchDev(int index){};
        void quit(){};
        void flash(){};
        void home(){};
        void stopm(){};
        void startm(){};
        void startD1(){};
        void startD2(){};
    
    public:
        int actual_channel;
        QMainWindow *main;
        QMenuBar *menuBar;
        QMenu *control_menu;
        QMenu *device_menu;
        QMenu *help_menu;
        QMenu *exit;
        QAction *exit_action;
        QAction **device_switch_actions;
        QAction *move_opt_action;
        QAction *device_opt_action;
        QAction *help_action;
        QAction *doc_action;
        QLabel *dev_name;
        QLabel *dev_ser;
        QLabel *serial;
        QRadioButton *chan_1;
        QRadioButton *chan_2;
        QRadioButton *chan_3;
        QPushButton *flash_button;
        QAction *flash_action;
        QPushButton *home_button;
        QAction *home_action;
        QPushButton *forward;
        QAction *forward_action;
        QPushButton *backward;
        QAction *backward_action;
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
        QAction *start_action;
        QPushButton *stop;
        QAction *stop_action;
        QGroupBox *status_box;
        QGridLayout *lstat;
        QLabel *moving;
        QLabel *homing;
        QLabel *stopping;
        QLabel *position;
        QLabel *velocity;
        QLabel *enc_count;
                    
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
            control_menu = new QMenu("Control",menuBar);
            device_menu = new QMenu("Device",menuBar);
            help_menu = new QMenu(menuBar);
            help_menu->setTitle("Help");
            exit = new QMenu("Exit",main);
            exit_action = new QAction("Exit",main);
            exit->addAction(exit_action);
            connect(exit_action, &QAction::triggered, this, &GUI::quit);
            menuBar->addMenu(control_menu);
            device_opt_action = new QAction("Device settings", main);
            move_opt_action = new QAction("Move options", main);
            control_menu->addAction(device_opt_action);
            control_menu->addAction(move_opt_action);
            connect(device_opt_action, &QAction::triggered, this, &GUI::openDevOptions);
            connect(move_opt_action, &QAction::triggered, this, &GUI::openMoveOptions);
            menuBar->addMenu(device_menu);
            device_switch_actions = new QAction*[devices_connected];
            for (unsigned int i =0; i < devices_connected; i++){
                std::string dev_label = std::to_string(i+1);
                dev_label.append(": ");
                dev_label.append(connected_device->SN);
                device_switch_actions[i] = new QAction(dev_label.c_str(),main);
                connect(move_opt_action, &QAction::triggered, this, [this](int i){ switchDev(i); });
            }
            menuBar->addMenu(help_menu);
            help_action = new QAction("Help",main);
            doc_action = new QAction("Documentation",main);
            help_menu->addAction(help_action);
            help_menu->addAction(doc_action);
            connect(help_action, &QAction::triggered, this, &GUI::openHelp);
            connect(doc_action, &QAction::triggered, this, &GUI::openDoc);
            menuBar->addMenu(exit);
                      
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
            actual_channel = 0;
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
            flash_button->setGeometry(40,350,100,50);
            flash_button->setText("Flash LED");
            flash_button->addAction(flash_action);
            flash_action = new QAction(main);
            connect(flash_action, &QAction::triggered, this, &GUI::flash);
            home_button = new QPushButton(main);
            home_button->setGeometry(150,350,100,50);
            home_button->setText("Home");
            home_button->addAction(home_action);
            home_action = new QAction(main);
            connect(home_action, &QAction::triggered, this, &GUI::home);
            
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
            forward->addAction(forward_action);
            forward_action = new QAction(main);
            connect(forward_action, &QAction::triggered, this, &GUI::startD1);
            backward = new QPushButton(main);
            backward->setIcon(QIcon("./src/triangle_down.png"));
            backward->setIconSize(QSize(65,65));
            backward->setGeometry(400,180,70,70);
            backward->addAction(backward_action);
            backward_action = new QAction(main);
            connect(backward_action, &QAction::triggered, this, &GUI::startD2);
            
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
            start->resize(50,40);
            start->addAction(start_action);
            start_action = new QAction(main);
            connect(start_action, &QAction::triggered, this, &GUI::startm);
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
            
            //stop button
            stop = new QPushButton("Stop",main);
            stop->setGeometry(275,380,100,50);
            stop->addAction(stop_action);
            stop_action = new QAction(main);
            connect(stop_action, &QAction::triggered, this, &GUI::stopm);
            
            //status bar
            status_box = new QGroupBox("Status",main);
            lstat = new QGridLayout();
            font.setPointSize(11);
            status_box->setFont(font);
            moving = new QLabel("Moving");
            homing = new QLabel("Homing");
            stopping = new QLabel("Stopping");
            std::string pos = "Position: ";
            pos.append(std::to_string(opened_device.motor[actual_channel].status_position));
            std::string encc = "Encoder counter: ";
            std::string vel = "Velocity: ";
            position = new QLabel(pos.c_str(),main);
            if (opened_device.enc_counter == 1) {
                encc.append(std::to_string(opened_device.motor[actual_channel].status_enc_count));
                vel.append("Unknown");
            }
            else {
                vel.append(std::to_string(opened_device.motor[actual_channel].status_velocity));
                encc.append("Unknown");
            }
            enc_count = new QLabel(encc.c_str(),main);
            velocity = new QLabel(vel.c_str(),main);
            
            homing->setEnabled(opened_device.motor[actual_channel].homing);
            moving->setEnabled(opened_device.motor[actual_channel].moving);
            stopping->setEnabled(opened_device.motor[actual_channel].stopping);
            lstat->addWidget(moving,0,0);
            lstat->addWidget(homing,0,1);
            lstat->addWidget(stopping,0,2);
            lstat->addWidget(position,1,0);
            lstat->addWidget(velocity,1,1);
            lstat->addWidget(enc_count,1,2);
            status_box->setLayout(lstat);
            status_box->setGeometry(10,450,780,140);
            
            
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

