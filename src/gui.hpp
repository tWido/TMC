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
#include <QtWidgets/QCheckBox>
#include <QPixmap>
#include <QIcon>
#include <QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QVBoxLayout>
#include <QPalette>

#include "api_calls.hpp"
#include "device.hpp"

#define GET_DEV_MESSAGE(mess_type, call)                            \
        mess_type* mess = (mess_type*) malloc(sizeof(mess_type));   \
        device_calls::call(mess);                                                 

class DevOpt: public QWidget{
    public slots:
        void setLedP(){};
        void ledDefaults(){};
        void setButtP(){};
        void getButtP(){};
        
    public:    
        QGridLayout *ledpl;
        QGroupBox *ledp;
        QCheckBox *lp1;
        QCheckBox *lp2;
        QCheckBox *lp3;
        QPushButton *ledset;
        QPushButton *leddef;
        QAction *ledset_action;
        QAction *leddef_action;
        QGroupBox *buttp;
        QGridLayout *buttpl;
        QRadioButton *gotoopt;
        QRadioButton *jogopt;
        QLineEdit *pos1;
        QLineEdit *pos2;
        QLineEdit *timeout;
        QLabel *pos1l;
        QLabel *pos2l;
        QLabel *timeoutl;
        QPushButton *getbuttp;
        QPushButton *setbuttp;
        QAction *setbuttp_action;
        QAction *getbuttp_action;
        
        void Setup(){
            this->setWindowTitle("Device setting");
            this->resize(600,400);
            this->setMinimumSize(600,400);
            this->setMaximumSize(600,400);
            QFont font = this->font();
            font.setPointSize(10);
            this->setFont(font);
            
            //led params box
            ledpl = new QGridLayout(this);
            ledp = new QGroupBox("LED parameters",this);
            lp1 = new QCheckBox("Flash on request",this);
            lp2 = new QCheckBox("Flash when limit is reached",this);
            lp3 = new QCheckBox("Lit while moving",this);
            ledset = new QPushButton("Set",this);
            leddef = new QPushButton("Defaults",this);
            GET_DEV_MESSAGE(GetLedMode,GetLedP)
            int mode = mess->GetMode();
            free(mess);
            lp1->setChecked(mode & 1);
            lp2->setChecked(mode & 2);
            lp3->setChecked(mode & 8);
            ledpl->addWidget(lp1,0,0);
            ledpl->addWidget(lp2,0,1);
            ledpl->addWidget(lp3,0,2);
            ledpl->addWidget(ledset,1,0);
            ledpl->addWidget(leddef,1,2);
            ledset_action = new QAction(this);
            leddef_action = new QAction(this);
            connect(ledset_action, &QAction::triggered, this, &DevOpt::setLedP);
            connect(leddef_action, &QAction::triggered, this, &DevOpt::ledDefaults);
            ledset->addAction(ledset_action);
            leddef->addAction(leddef_action);
            ledp->setLayout(ledpl);
            ledp->setGeometry(10,10,580,190);
            
            //button param box
            buttpl = new QGridLayout(this);
            buttp = new QGroupBox("Buttons parameters",this);
            gotoopt = new QRadioButton("Goto positions",this);
            jogopt = new QRadioButton("Jog",this);
            pos1 = new QLineEdit(this);
            pos2 = new QLineEdit(this);
            timeout = new QLineEdit(this);
            pos1->setInputMask("999999999");
            pos2->setInputMask("999999999");
            timeout->setInputMask("999999");
            pos1l = new QLabel("Position 1",this);
            pos2l = new QLabel("Position 2",this);
            timeoutl = new QLabel("Timeout", this);
            getbuttp = new QPushButton("Set",this);
            setbuttp = new QPushButton("Get",this);
            setbuttp_action = new QAction(this);
            getbuttp_action = new QAction(this);
            connect(setbuttp_action, &QAction::triggered, this, &DevOpt::setButtP);
            connect(getbuttp_action, &QAction::triggered, this, &DevOpt::getButtP);
            setbuttp->addAction(setbuttp_action);
            getbuttp->addAction(getbuttp_action);
            buttpl->addWidget(jogopt, 0, 0);
            buttpl->addWidget(pos1l,1,1);
            buttpl->addWidget(pos2l,1,3);
            buttpl->addWidget(timeoutl,1,5);
            buttpl->addWidget(gotoopt,1,0);
            buttpl->addWidget(pos1,1,2);
            buttpl->addWidget(pos2,1,4);
            buttpl->addWidget(timeout,1,6);
            buttpl->addWidget(setbuttp,3,2);
            buttpl->addWidget(getbuttp,3,3);
            buttp->setLayout(buttpl);
            buttp->setGeometry(10,200,580,190);
            
            this->show();
        } 

};

class MovOpt: public QWidget{
    public slots:
        void setHVel(){};
        void getHVel(){};
        void setBDist(){};
        void getBDist(){};
        void setAccP(){};
        void getAccP(){};
        void setPowerP(){};
        void getPowerP(){};

    public:       
        QGroupBox *hvel_box;
        QHBoxLayout *hvel_layout;
        QLabel *hvel_label;
        QLineEdit *hvel_edit;
        QPushButton *hvel_get;
        QPushButton *hvel_set;
        QAction *hvel_set_action;
        QAction *hvel_get_action;
        QGroupBox *bdist_box;
        QHBoxLayout *bdist_layout;
        QLabel *bdist_label;
        QLineEdit *bdist_edit;
        QPushButton *bdist_get;
        QPushButton *bdist_set;
        QAction *bdist_set_action;
        QAction *bdist_get_action;
        QGroupBox *accp_box;
        QHBoxLayout *accp_layout;
        QLabel *accp_label;
        QLineEdit *accp_edit;
        QPushButton *accp_get;
        QPushButton *accp_set;
        QAction *accp_set_action;
        QAction *accp_get_action;
        QGroupBox *powerp_box;
        QHBoxLayout *powerp_layout;
        QLabel *powerp1_label;
        QLineEdit *powerp1_edit;
        QLabel *powerp2_label;
        QLineEdit *powerp2_edit;
        QPushButton *powerp_get;
        QPushButton *powerp_set;
        QAction *powerp_set_action;
        QAction *powerp_get_action;
        QGroupBox *jopg;
        QGridLayout *jogpl;


        void Setup(int index){
            this->setWindowTitle("Move options");
            this->resize(500,400);
            this->setMaximumSize(500,400);
            this->setMinimumSize(500,400);
            QFont font = this->font();
            font.setPointSize(10);
            this->setFont(font);
            
            // Homing velocity
            hvel_box = new QGroupBox("Homing velocity",this);
            hvel_layout = new QHBoxLayout(this);
            hvel_label = new QLabel("Velocity",this);
            hvel_edit = new QLineEdit(this);
            hvel_edit->setInputMask("999999");
            hvel_get = new QPushButton("Get",this);
            hvel_set = new QPushButton("Set",this);
            hvel_get_action = new QAction(this);
            hvel_set_action = new QAction(this);
            connect(hvel_get_action, &QAction::triggered, this, &MovOpt::getHVel);
            connect(hvel_set_action, &QAction::triggered, this, &MovOpt::setHVel);
            hvel_get->addAction(hvel_get_action);
            hvel_set->addAction(hvel_set_action);
            hvel_layout->addWidget(hvel_label);
            hvel_layout->addWidget(hvel_edit);
            hvel_layout->addWidget(hvel_get);
            hvel_layout->addWidget(hvel_set);
            hvel_box->setLayout(hvel_layout);
            hvel_box->setGeometry(10,10,480,85);
            
            //Backlash distance
            bdist_box = new QGroupBox("Backlash Distance",this);
            bdist_layout = new QHBoxLayout(this);
            bdist_label = new QLabel("Distance",this);
            bdist_edit = new QLineEdit(this);
            bdist_edit->setInputMask("999999");
            bdist_get = new QPushButton("Get",this);
            bdist_set = new QPushButton("Set",this);
            bdist_get_action = new QAction(this);
            bdist_set_action = new QAction(this);
            connect(bdist_get_action, &QAction::triggered, this, &MovOpt::getBDist);
            connect(bdist_set_action, &QAction::triggered, this, &MovOpt::setBDist);
            bdist_get->addAction(bdist_get_action);
            bdist_set->addAction(bdist_set_action);
            bdist_layout->addWidget(bdist_label);
            bdist_layout->addWidget(bdist_edit);
            bdist_layout->addWidget(bdist_get);
            bdist_layout->addWidget(bdist_set);
            bdist_box->setLayout(bdist_layout);
            bdist_box->setGeometry(10,100,480,100);
            
            //Acceleration profile
            accp_box = new QGroupBox("Acceleration profile",this);
            accp_layout = new QHBoxLayout(this);
            accp_label = new QLabel("Profile (0-18)",this);
            accp_edit = new QLineEdit(this);
            accp_edit->setInputMask("99");
            accp_get = new QPushButton("Get",this);
            accp_set = new QPushButton("Set",this);
            accp_get_action = new QAction(this);
            accp_set_action = new QAction(this);
            connect(accp_get_action, &QAction::triggered, this, &MovOpt::getAccP);
            connect(accp_set_action, &QAction::triggered, this, &MovOpt::setAccP);
            accp_get->addAction(accp_get_action);
            accp_set->addAction(accp_set_action);
            accp_layout->addWidget(accp_label);
            accp_layout->addWidget(accp_edit);
            accp_layout->addWidget(accp_get);
            accp_layout->addWidget(accp_set);
            accp_box->setLayout(accp_layout);
            accp_box->setGeometry(10,205,480,100);
            
            //Power parameters
            powerp_box = new QGroupBox("Power parameters",this);
            powerp_layout = new QHBoxLayout(this);
            powerp1_label = new QLabel("Move power in %",this);
            powerp1_edit = new QLineEdit(this);
            powerp1_edit->setInputMask("999");
            powerp2_label = new QLabel("Rest power in %",this);
            powerp2_edit = new QLineEdit(this);
            powerp2_edit->setInputMask("999");
            powerp_get = new QPushButton("Get",this);
            powerp_set = new QPushButton("Set",this);
            powerp_get_action = new QAction(this);
            powerp_set_action = new QAction(this);
            connect(powerp_get_action, &QAction::triggered, this, &MovOpt::getPowerP);
            connect(powerp_set_action, &QAction::triggered, this, &MovOpt::setPowerP);
            powerp_get->addAction(powerp_get_action);
            powerp_set->addAction(powerp_set_action);
            powerp_layout->addWidget(powerp1_label);
            powerp_layout->addWidget(powerp1_edit);
            powerp_layout->addWidget(powerp2_label);
            powerp_layout->addWidget(powerp2_edit);
            powerp_layout->addWidget(powerp_get);
            powerp_layout->addWidget(powerp_set);
            powerp_box->setLayout(powerp_layout);
            powerp_box->setGeometry(10,310,480,100);
            
            //Jog parameters
            
            this->show();
        }

};

class GUI: public QMainWindow{
    
    public slots:
        void openDevOptions(){
            devOpt = new DevOpt();
            devOpt->Setup();
        };
        void openMoveOptions(){
            moveOpt = new MovOpt();
            moveOpt->Setup(GetChan());
        };
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
        MovOpt *moveOpt;
        DevOpt *devOpt;
                    
        int GetChan(){ 
            return 0;
        } 
        
        void Setup(){
            //main
            this->resize(800, 600);
            this->setWindowTitle("Thorlabs motor controller");
            this->setMinimumSize(800,600);
            this->setMaximumSize(800,600);

            //Menu
            menuBar = new QMenuBar(this);
            menuBar->setGeometry(0, 0, 800, 40);
            menuBar->move(0,0);
            this->setMenuBar(menuBar);
            control_menu = new QMenu("Control",menuBar);
            device_menu = new QMenu("Device",menuBar);
            help_menu = new QMenu(menuBar);
            help_menu->setTitle("Help");
            exit = new QMenu("Exit",this);
            exit_action = new QAction("Exit",this);
            connect(exit_action, &QAction::triggered, this, &GUI::quit);
            exit->addAction(exit_action);
            menuBar->addMenu(control_menu);
            device_opt_action = new QAction("Device settings", this);
            move_opt_action = new QAction("Move options", this);
            connect(device_opt_action, &QAction::triggered, this, &GUI::openDevOptions);
            connect(move_opt_action, &QAction::triggered, this, &GUI::openMoveOptions);
            control_menu->addAction(device_opt_action);
            control_menu->addAction(move_opt_action);
            menuBar->addMenu(device_menu);
            device_switch_actions = new QAction*[devices_connected];
            for (unsigned int i =0; i < devices_connected; i++){
                std::string dev_label = std::to_string(i+1);
                dev_label.append(": ");
                dev_label.append(connected_device->SN);
                device_switch_actions[i] = new QAction(dev_label.c_str(),this);
                connect(move_opt_action, &QAction::triggered, this, [this](int i){ switchDev(i); });
            }
            menuBar->addMenu(help_menu);
            help_action = new QAction("Help",this);
            doc_action = new QAction("Documentation",this);
            connect(help_action, &QAction::triggered, this, &GUI::openHelp);
            connect(doc_action, &QAction::triggered, this, &GUI::openDoc);
            help_menu->addAction(help_action);
            help_menu->addAction(doc_action);
            menuBar->addMenu(exit);
                      
            //Device labels
            dev_name = new QLabel(this);
            std::string dname = "Device: ";
            QFont font = dev_name->font();
            font.setPointSize(14);
            dev_name->setFont(font);
            dname.append(opened_device.dev_type_name);
            dev_name->setText(dname.c_str());
            dev_name->setGeometry(20,40,250,30);
            dev_ser = new QLabel(this);
            font = dev_ser->font();
            font.setPointSize(14);
            dev_ser->setFont(font);
            std::string serial = "SN: ";
            serial.append(opened_device.SN);
            dev_ser->setGeometry(20,65,250,30);
            dev_ser->setText(serial.c_str());
            
            //channel switch
            channels = new QGroupBox("Channels",this);
            chan_box = new QVBoxLayout();
            font.setPointSize(12);
            channels->setFont(font);
            chan_1 = new QRadioButton(this);
            chan_1->resize(100,50);
            chan_1->setText("Channel 1");
            chan_1->setChecked(true);
            actual_channel = 0;
            chan_1->setFont(font);           
            chan_2 = new QRadioButton(this);
            chan_2->resize(100,50);
            chan_2->setText("Channel 2");
            chan_2->setFont(font);
            chan_2->setChecked(false);
            if (opened_device.channels == 2 || (opened_device.bays >=2 && opened_device.bay_used[1])){
                chan_2->setDisabled(false);
            }
            else chan_2->setDisabled(true);
            chan_3 = new QRadioButton(this);
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
            flash_button = new QPushButton(this);
            flash_button->setGeometry(40,350,100,50);
            flash_button->setText("Flash LED");
            flash_action = new QAction(this);
            connect(flash_action, &QAction::triggered, this, &GUI::flash);
            flash_button->addAction(flash_action);
            home_button = new QPushButton(this);
            home_button->setGeometry(150,350,100,50);
            home_button->setText("Home");
            home_action = new QAction(this);
            connect(home_action, &QAction::triggered, this, &GUI::home);
            home_button->addAction(home_action);
            
            //moves directional
            moves_l = new QLabel(this);
            moves_l->setText("Moves");
            moves_l->setGeometry(475,30,100,50);
            font.setPointSize(15);
            moves_l->setFont(font);
            forward = new QPushButton(this);
            forward->setIcon(QIcon("./src/triangle_up.png"));
            forward->setIconSize(QSize(65,65));
            forward->setGeometry(400,100,70,70);
            forward_action = new QAction(this);
            connect(forward_action, &QAction::triggered, this, &GUI::startD1);
            forward->addAction(forward_action);
            backward = new QPushButton(this);
            backward->setIcon(QIcon("./src/triangle_down.png"));
            backward->setIconSize(QSize(65,65));
            backward->setGeometry(400,180,70,70);
            backward_action = new QAction(this);
            connect(backward_action, &QAction::triggered, this, &GUI::startD2);
            backward->addAction(backward_action);
            
            dir_moves = new QGroupBox("Directional",this);
            ldir_moves = new QVBoxLayout();
            font.setPointSize(12);
            dir_moves->setFont(font);
            jogm = new QRadioButton("Jog",this);
            jogm->resize(100,50);
            velm = new QRadioButton("Velocity",this);
            velm->resize(100,50);
            ldir_moves->addWidget(jogm);
            ldir_moves->addWidget(velm);
            ldir_moves->setSpacing(8);
            dir_moves->setLayout(ldir_moves);
            dir_moves->setGeometry(500,80,150,175);
            
            //moves non-directional
            moves = new QGroupBox(this);
            lmoves = new QGridLayout();
            absm = new QRadioButton("Absolute",this);
            absm->resize(100, 50);
            abspl = new QLabel("Position",this);
            abspl->resize(50,50);
            abspos = new QLineEdit(this);
            abspos->setInputMask("999999999");
            abspos->setText("0");
            abspos->resize(80,25);
            reldist = new QLineEdit(this);
            reldist->setText("0");
            reldist->setInputMask("999999999");
            reldist->resize(80,25);
            relm = new QRadioButton("Relative",this);
            relm->resize(100, 50);
            reldl = new QLabel("Distance",this);
            reldl->resize(50,50);
            start = new QPushButton("Start",this);
            start->resize(50,40);
            start_action = new QAction(this);
            connect(start_action, &QAction::triggered, this, &GUI::startm);
            start->addAction(start_action);
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
            stop = new QPushButton("Stop",this);
            stop->setGeometry(275,380,100,50);
            stop_action = new QAction(this);
            connect(stop_action, &QAction::triggered, this, &GUI::stopm);
            stop->addAction(stop_action);      
            
            //status bar
            status_box = new QGroupBox("Status",this);
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
            position = new QLabel(pos.c_str(),this);
            if (opened_device.enc_counter == 1) {
                encc.append(std::to_string(opened_device.motor[actual_channel].status_enc_count));
                vel.append("Unknown");
            }
            else {
                vel.append(std::to_string(opened_device.motor[actual_channel].status_velocity));
                encc.append("Unknown");
            }
            enc_count = new QLabel(encc.c_str(),this);
            velocity = new QLabel(vel.c_str(),this);
            
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
            
            
            this->show();
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

