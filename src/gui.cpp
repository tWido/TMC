#include "gui.hpp"
#include "api.hpp"
#include "device.hpp"

 int run_gui(){
    char **argv = NULL;
    int argc = 0;
    QApplication a( argc, argv );
    GUI gui;
    gui.Setup();
    
    return a.exec();
}
 
#define ERROR_DIALOG(mess)                              \
        error_window = new QDialog(this);               \
        QLabel *err_mess = new QLabel(mess,error_window);\
        err_mess->move(40,40);                          \
        error_window->resize(350,100);                  \
        error_window->show(); 
 
#define CHAN_SELECTED(command, params)                                          \
            if (opened_device.bays == -1 ){                                     \
                if (chan_1->isChecked()) device_calls::command(params, 0x50,1); \
                if (chan_2->isChecked()) device_calls::command(params, 0x50,2); \
                if (chan_3->isChecked()) device_calls::command(params, 0x50,3); \
            }                                                                   \
            else {                                                              \
                if (chan_1->isChecked()) device_calls::command(params, 0x21);   \
                if (chan_2->isChecked()) device_calls::command(params, 0x22);   \
                if (chan_3->isChecked()) device_calls::command(params, 0x23);   \
            }

void DevOpt::Setup(){
    this->setWindowTitle("Device setting");
    this->resize(600,400);
    this->setMinimumSize(600,400);
    this->setMaximumSize(600,400);
    QFont font = this->font();
    font.setPointSize(10);
    this->setFont(font);

    //led params box
    ledp = new QGroupBox("LED parameters",this);
    ledpl = new QGridLayout(ledp);
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
    connect(ledset, &QPushButton::clicked, 
        [this]{
            uint8_t mode = 0;
            if (this->lp1->isChecked()) mode +=1;
            if (this->lp1->isChecked()) mode +=2;
            if (this->lp1->isChecked()) mode +=8;
            device_calls::SetLedP(mode);
        } 
    );

    connect(leddef, &QPushButton::clicked, 
        [this]{
            this->lp1->setChecked(true);
            this->lp2->setChecked(true);
            this->lp3->setChecked(true);
        } 
    );
    ledp->setLayout(ledpl);
    ledp->setGeometry(10,10,580,150);

    //button param box
    buttp = new QGroupBox("Buttons parameters",this);
    buttpl = new QGridLayout(buttp);
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
    connect(setbuttp, &QPushButton::clicked, 
        [this]{
            uint16_t mode = 0;
            if (gotoopt->isChecked()) mode = 2;
            else mode = 1;
            int32_t position1 = std::stoi(this->pos1->text().toStdString(),0,10);
            int32_t position2 = std::stoi(this->pos2->text().toStdString(),0,10);
            uint16_t timeout_val = std::stoi(this->timeout->text().toStdString(),0,10);
            device_calls::SetButtons(mode, position1, position2, timeout_val);
        } 
    );
    connect(getbuttp, &QPushButton::clicked, 
        [this]{
            GET_DEV_MESSAGE(GetButtonParams,GetButtonsInfo)
            if (mess->GetMode() == 1) this->jogopt->setChecked(true);
            if (mess->GetMode() == 2){ 
                this->gotoopt->setChecked(true);
                std::stringstream sts ;
                sts << mess->GetPosition1(); 
                this->pos1->setText(sts.str().c_str());
                sts << mess->GetPosition2(); 
                this->pos2->setText(sts.str().c_str());
                sts << mess->GetTimeout(); 
                this->timeout->setText(sts.str().c_str());
            }
            free(mess);
        } 
    );
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
    buttp->setGeometry(10,160,580,190);

    this->show();
} 

void MovOpt::Setup(int index){
    chan_id = index;
    this->setWindowTitle("Move options");
    this->resize(500,700);
    this->setMaximumSize(500,700);
    this->setMinimumSize(500,700);
    QFont font = this->font();
    font.setPointSize(10);
    this->setFont(font);

    // Homing velocity
    hvel_box = new QGroupBox("Homing velocity",this);
    hvel_layout = new QHBoxLayout(hvel_box);
    hvel_label = new QLabel("Velocity",this);
    hvel_edit = new QLineEdit(this);
    hvel_edit->setInputMask("999999");
    hvel_get = new QPushButton("Get",this);
    hvel_set = new QPushButton("Set",this);
    connect(hvel_get, &QPushButton::clicked, [this]{} );
    connect(hvel_set, &QPushButton::clicked, [this]{} );
    hvel_layout->addWidget(hvel_label);
    hvel_layout->addWidget(hvel_edit);
    hvel_layout->addWidget(hvel_get);
    hvel_layout->addWidget(hvel_set);
    hvel_box->setLayout(hvel_layout);
    hvel_box->setGeometry(10,10,350,85);

    //Backlash distance
    bdist_box = new QGroupBox("Backlash Distance",this);
    bdist_layout = new QHBoxLayout(bdist_box);
    bdist_label = new QLabel("Distance",this);
    bdist_edit = new QLineEdit(this);
    bdist_edit->setInputMask("999999");
    bdist_get = new QPushButton("Get",this);
    bdist_set = new QPushButton("Set",this);
    connect(bdist_get, &QPushButton::clicked, 
        [this]{
            std::stringstream sts;
            GET_DEV_MESSAGE(GetGeneralMoveParams, GetBacklashDist)
            sts << mess->GetBacklashDist();
            bdist_edit->setText(sts.str().c_str());
            free(mess);
        } 
    );
    connect(bdist_set, &QPushButton::clicked, 
        [this]{
            int32_t dist =std::stoi(this->bdist_edit->text().toStdString(),0,10);
            if (opened_device.bays == -1 )
                device_calls::SetBacklashDist(dist, 0x50, chan_id);
            else device_calls::SetBacklashDist(dist, chan_id+0x20);
        } 
    );
    bdist_layout->addWidget(bdist_label);
    bdist_layout->addWidget(bdist_edit);
    bdist_layout->addWidget(bdist_get);
    bdist_layout->addWidget(bdist_set);
    bdist_box->setLayout(bdist_layout);
    bdist_box->setGeometry(10,100,350,100);

    //Acceleration profile
    accp_box = new QGroupBox("Acceleration profile",this);
    accp_layout = new QHBoxLayout(accp_box);
    accp_label = new QLabel("Profile (0-18)",this);
    accp_edit = new QLineEdit(this);
    accp_edit->setInputMask("99");
    accp_get = new QPushButton("Get",this);
    accp_set = new QPushButton("Set",this);
    connect(accp_get, &QPushButton::clicked, 
        [this]{
            std::stringstream sts;
            GET_DEV_MESSAGE(GetBowIndex, GetAccelerationProfile)
            sts << mess->BowIndex();
            bdist_edit->setText(sts.str().c_str());
            free(mess);
        } 
    );
    connect(accp_set, &QPushButton::clicked, 
        [this]{
            uint16_t profile =std::stoi(this->accp_edit->text().toStdString(),0,10);
            int ret = 0;
            if (opened_device.bays == -1 ) ret = device_calls::SetAccelerationProfile(profile, 0x50, chan_id);
            else  ret = device_calls::SetAccelerationProfile(profile, chan_id+0x20);
            if (ret ==INVALID_PARAM_1 ){ ERROR_DIALOG("ERROR: non existing value given") }
        } 
    );
    accp_layout->addWidget(accp_label);
    accp_layout->addWidget(accp_edit);
    accp_layout->addWidget(accp_get);
    accp_layout->addWidget(accp_set);
    accp_box->setLayout(accp_layout);
    accp_box->setGeometry(10,205,350,100);

    //Power parameters
    powerp_box = new QGroupBox("Power parameters",this);
    powerp_layout = new QHBoxLayout(powerp_box);
    powerp1_label = new QLabel("Move power in %",this);
    powerp1_edit = new QLineEdit(this);
    powerp1_edit->setInputMask("999");
    powerp2_label = new QLabel("Rest power in %",this);
    powerp2_edit = new QLineEdit(this);
    powerp2_edit->setInputMask("999");
    powerp_get = new QPushButton("Get",this);
    powerp_set = new QPushButton("Set",this);
    connect(powerp_get, &QPushButton::clicked, 
        [this]{
            std::stringstream sts;
            GET_DEV_MESSAGE(GetPowerParams, GetPowerUsed)
            sts << mess->GetRestFactor();
            powerp1_edit->setText(sts.str().c_str());
            sts << mess->GetMoveFactor();
            powerp2_edit->setText(sts.str().c_str());
            free(mess);
        } 
    );
    connect(powerp_set, &QPushButton::clicked, 
        [this]{
            uint16_t restf = std::stoi(this->powerp1_edit->text().toStdString(),0,10);
            uint16_t movef = std::stoi(this->powerp2_edit->text().toStdString(),0,10);
            int ret = 0;
            if (opened_device.bays == -1 ) ret = device_calls::SetPowerUsed(restf, movef, 0x50, chan_id);
            else  ret = device_calls::SetPowerUsed(restf, movef, chan_id+0x20);
            if (ret ==INVALID_PARAM_1 || ret ==INVALID_PARAM_2 ){ ERROR_DIALOG("ERROR: invalid parameter given") }     
        } 
    );
    powerp_layout->addWidget(powerp1_label);
    powerp_layout->addWidget(powerp1_edit);
    powerp_layout->addWidget(powerp2_label);
    powerp_layout->addWidget(powerp2_edit);
    powerp_layout->addWidget(powerp_get);
    powerp_layout->addWidget(powerp_set);
    powerp_box->setLayout(powerp_layout);
    powerp_box->setGeometry(10,310,480,100);

    //Jog parameters
    jogp_box = new QGroupBox(this);
    jogp_layout = new QGridLayout(jogp_box);
    jogp_mode = new QGroupBox("Mode",this);
    jogp_model = new QVBoxLayout(jogp_mode);
    jogp_mode1 = new QRadioButton("Continuous",this);
    jogp_mode2 = new QRadioButton("Single step",this);
    jogp_model->addWidget(jogp_mode1);
    jogp_model->addWidget(jogp_mode2);
    jogp_mode->setLayout(jogp_model);
    jogp_stopmode = new QGroupBox("Stop mode",this);
    jogp_stopmodel = new QVBoxLayout(jogp_stopmode);
    jogp_stopmode1 = new QRadioButton("Immediate stop",this);
    jogp_stopmode2 = new QRadioButton("Profiled stop",this);
    jogp_stopmodel->addWidget(jogp_stopmode1);
    jogp_stopmodel->addWidget(jogp_stopmode2);
    jogp_stopmode->setLayout(jogp_stopmodel);
    jogp_vell = new QLabel("Velocity",this);
    jogp_vele = new QLineEdit(this);
    jogp_vele->setInputMask("999999");
    jogp_accl = new QLabel("Acceleration",this);
    jogp_acce = new QLineEdit(this);
    jogp_acce->setInputMask("999999");
    jogp_stepl = new QLabel("Step size",this);
    jogp_stepe = new QLineEdit(this);
    jogp_stepe->setInputMask("999999");
    jogp_get = new QPushButton("Get",this);
    jogp_set = new QPushButton("Set",this);
    connect(jogp_get, &QPushButton::clicked, 
        [this]{
            std::stringstream sts;
            GET_DEV_MESSAGE(GetJogParams, GetJogP)
            if ( mess->GetJogMode() == 1 ) jogp_mode1->setChecked(true);
            else {
                jogp_mode2->setChecked(true);
                sts << mess->GetStepSize();
                jogp_stepe->setText(sts.str().c_str());
                sts<< mess->GetMaxVel();
                jogp_vele->setText(sts.str().c_str());
                sts<< mess->GetAcceleration();
                jogp_acce->setText(sts.str().c_str());
            }
            if (mess->GetStopMode() == 1) jogp_stopmode1->setChecked(true);
            else jogp_stopmode2->setChecked(true);
            free(mess);
        } 
    );
    connect(jogp_set, &QPushButton::clicked, 
        [this]{
            uint16_t mode, stopmode;
            if (jogp_mode1->isChecked())  mode = 1; 
            else mode = 2;
            int32_t stepSize = std::stoi(this->jogp_stepe->text().toStdString(),0,10);;  
            int32_t vel = std::stoi(this->jogp_vele->text().toStdString(),0,10);;
            int32_t acc = std::stoi(this->jogp_acce->text().toStdString(),0,10);; 
            if (jogp_stopmode1->isChecked())  stopmode = 1; 
            else stopmode = 2;
            int ret = 0;
            if (opened_device.bays == -1 ) ret = device_calls::SetJogP(mode, stepSize, vel, acc, stopmode, 0x50, chan_id);
            else ret = device_calls::SetJogP(mode, stepSize, vel, acc, stopmode, chan_id+0x20);
            if (ret ==INVALID_PARAM_1 ){ ERROR_DIALOG("ERROR: velocity exceeds restriction")}
            else if (ret ==INVALID_PARAM_2 ){ ERROR_DIALOG("ERROR: acceleration exceeds restriction")} 
        } 
    );
    jogp_layout->addWidget(jogp_mode,0,0,3,1);
    jogp_layout->addWidget(jogp_stopmode, 0,1,3,1);
    jogp_layout->addWidget(jogp_vell,0 ,2);
    jogp_layout->addWidget(jogp_vele,0 ,3);
    jogp_layout->addWidget(jogp_accl,1 ,2);
    jogp_layout->addWidget(jogp_acce,1 ,3);
    jogp_layout->addWidget(jogp_stepl,2 ,2);
    jogp_layout->addWidget(jogp_stepe,2 ,3);
    jogp_layout->addWidget(jogp_get,3,0);
    jogp_layout->addWidget(jogp_set,3,1);
    jogp_box->setLayout(jogp_layout);
    jogp_box->setGeometry(10,410,480,250);

    this->show();
}

void GUI::Setup(){
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
    menuBar->addMenu(control_menu);
    device_opt_action = new QAction("Device settings", this);
    move_opt_action = new QAction("Move options", this);
    connect(device_opt_action, &QAction::triggered, this, &GUI::openDevOptions);
    connect(move_opt_action, &QAction::triggered, this, &GUI::openMoveOptions);
    control_menu->addAction(device_opt_action);
    control_menu->addAction(move_opt_action);
    menuBar->addMenu(device_menu);
    device_switch_actions = new QAction*[devices_connected];
    for (int i =0; i < devices_connected; i++){
        std::string dev_label = std::to_string(i+1);
        dev_label.append(": ");
        dev_label.append(connected_device->SN);
        device_switch_actions[i] = new QAction(dev_label.c_str(),this);
        connect(move_opt_action, &QAction::triggered, this, [this, i]{ OpenDevice(i); });
        device_menu->addAction(device_switch_actions[i]);
    }
    menuBar->addMenu(help_menu);
    help_action = new QAction("Help",this);
    doc_action = new QAction("Documentation",this);
    connect(help_action, &QAction::triggered, this, &GUI::openHelp);
    connect(doc_action, &QAction::triggered, this, &GUI::openDoc);
    help_menu->addAction(help_action);
    help_menu->addAction(doc_action);

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
    chan_box = new QVBoxLayout(channels);
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
    connect(flash_button, &QPushButton::clicked,
        [this]{
            if (opened_device.bays == -1 )
                device_calls::Identify();
            else {
                if (chan_1->isChecked())device_calls::Identify(0x21);
                if (chan_2->isChecked())device_calls::Identify(0x22);
                if (chan_3->isChecked())device_calls::Identify(0x23);
            }
        }
    );
    home_button = new QPushButton(this);
    home_button->setGeometry(150,350,100,50);
    home_button->setText("Home");
    connect(home_button, &QPushButton::clicked,
        [this]{
            if (opened_device.bays == -1 ){
                if (chan_1->isChecked())device_calls::MoveToHome(0x50,1);
                if (chan_2->isChecked())device_calls::MoveToHome(0x50,2);
                if (chan_3->isChecked())device_calls::MoveToHome(0x50,3);
        }
            else {
                if (chan_1->isChecked())device_calls::MoveToHome(0x21);
                if (chan_2->isChecked())device_calls::MoveToHome(0x22);
                if (chan_3->isChecked())device_calls::MoveToHome(0x23);
            }
        }
    );

    //moves directional
    moves_l = new QLabel(this);
    moves_l->setText("Moves");
    moves_l->setGeometry(475,30,100,50);
    font.setPointSize(15);
    moves_l->setFont(font);
    forward = new QPushButton(this);
    forward->setIcon(QIcon("./src/triangle_up.png"));
    forward->setIconSize(QSize(65,65));
    forward->setGeometry(450,100,70,70);
    connect(forward, &QPushButton::clicked,
        [this]{
            if (jogm->isChecked()){ 
                CHAN_SELECTED(StartJogMove,1)
            }
            if (velm->isChecked()){ 
                CHAN_SELECTED(StartSetVelocityMove,1)
            }
        }
    );
    backward = new QPushButton(this);
    backward->setIcon(QIcon("./src/triangle_down.png"));
    backward->setIconSize(QSize(65,65));
    backward->setGeometry(450,180,70,70);
    connect(backward, &QPushButton::clicked,
        [this]{
            if (jogm->isChecked()){ 
                CHAN_SELECTED(StartJogMove,2)
            }
            if (velm->isChecked()){
                CHAN_SELECTED(StartSetVelocityMove,2)
            }
        }
    );

    dir_moves = new QGroupBox("Directional",this);
    ldir_moves = new QVBoxLayout(dir_moves);
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
    dir_moves->setGeometry(550,80,200,175);

    //moves non-directional
    moves = new QGroupBox(this);
    lmoves = new QGridLayout(moves);
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
    connect(start, &QPushButton::clicked,
        [this]{
            if (absm->isChecked()){
                int32_t pos = std::stoi(this->reldist->text().toStdString(),0,10);
                int ret = 0;
                if (opened_device.bays == -1 ){
                    if (chan_1->isChecked()) ret = device_calls:: StartAbsoluteMove ( pos , 0x50,1);
                    if (chan_2->isChecked()) ret = device_calls:: StartAbsoluteMove ( pos , 0x50,2);
                    if (chan_3->isChecked()) ret = device_calls:: StartAbsoluteMove ( pos , 0x50,3);
                }
                else {
                    if (chan_1->isChecked()) ret = device_calls:: StartAbsoluteMove ( pos , 0x21);
                    if (chan_2->isChecked()) ret = device_calls:: StartAbsoluteMove ( pos , 0x22);
                    if (chan_3->isChecked()) ret = device_calls:: StartAbsoluteMove ( pos , 0x23);
                }
                if (ret == INVALID_PARAM_1) {ERROR_DIALOG("ERROR: position exceeds restriction")}
            }
            if (relm->isChecked()){
                int32_t dist = std::stoi(this->abspos->text().toStdString(),0,10);
                CHAN_SELECTED(StartRelativeMove,dist)
            }
        } 
    );
    lmoves->setVerticalSpacing(4);
    lmoves->addWidget(absm,0,0);
    lmoves->addWidget(abspos,0,2);
    lmoves->addWidget(abspl,0,1);
    lmoves->addWidget(relm,1,0);
    lmoves->addWidget(reldist,1,2);
    lmoves->addWidget(reldl,1,1);
    lmoves->addWidget(start);
    moves->setLayout(lmoves);
    moves->setGeometry(450,250,300,200);

    //stop button
    stop = new QPushButton("Stop",this);
    stop->setGeometry(310,380,100,50);
    connect(stop, &QPushButton::clicked,
        [this]{
            CHAN_SELECTED(StopMovement,2)
        }
    );   

    //status bar
    status_box = new QGroupBox("Status",this);
    lstat = new QGridLayout(status_box);
    font.setPointSize(11);
    status_box->setFont(font);
    moving = new QLabel("Moving",this);
    homing = new QLabel("Homing",this);
    stopping = new QLabel("Stopping",this);
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
    
    //timer for status updates
    update_timer = new QTimer(this);
    connect(update_timer, &QTimer::timeout, this, 
        [this]{
            EmptyIncomingQueue();
            this->repaint();
        }
    );
    update_timer->start(1000);
    
    this->show();
}

void GUI::openDevOptions(){
    devOpt = new DevOpt();
    devOpt->Setup();
};
void GUI::openMoveOptions(){
    moveOpt = new MovOpt();
    if (chan_1->isChecked()) moveOpt->Setup(1);
    if (chan_2->isChecked()) moveOpt->Setup(2);
    if (chan_3->isChecked()) moveOpt->Setup(3);
};
void GUI::openHelp(){
    helpw = new HelpText();
    helpw->Setup();
};
void GUI::openDoc(){
    if (system("gnome-open ./docs/APT_Communications_Protocol_Rev_15.pdf") !=0){
        ERROR_DIALOG("ERROR: Default pdf viewer not set")
        fprintf(stderr,"Default pdf viewer not set\n");
    }
};
