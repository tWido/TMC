#ifndef GUI_HPP
#define	GUI_HPP

#include <string> 
#include <sstream>
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
#include <QPlainTextEdit>
#include <QTimer>
#include <QDialog>


#define GET_DEV_MESSAGE(mess_type, call)    \
        mess_type mess;                     \
        device_calls::call(mess);          

extern int run_gui();

class DevOpt: public QWidget{
    Q_OBJECT    
    public:    
        QGridLayout *ledpl;
        QGroupBox *ledp;
        QCheckBox *lp1;
        QCheckBox *lp2;
        QCheckBox *lp3;
        QPushButton *ledset;
        QPushButton *leddef;
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
        QDialog *error_window;
        
        void Setup();
};

class MovOpt: public QWidget{
    Q_OBJECT
    public:
        int chan_id;
        QGroupBox *hvel_box;
        QHBoxLayout *hvel_layout;
        QLabel *hvel_label;
        QLineEdit *hvel_edit;
        QPushButton *hvel_get;
        QPushButton *hvel_set;
        QGroupBox *bdist_box;
        QHBoxLayout *bdist_layout;
        QLabel *bdist_label;
        QLineEdit *bdist_edit;
        QPushButton *bdist_get;
        QPushButton *bdist_set;
        QGroupBox *accp_box;
        QHBoxLayout *accp_layout;
        QLabel *accp_label;
        QLineEdit *accp_edit;
        QPushButton *accp_get;
        QPushButton *accp_set;
        QGroupBox *powerp_box;
        QHBoxLayout *powerp_layout;
        QLabel *powerp1_label;
        QLineEdit *powerp1_edit;
        QLabel *powerp2_label;
        QLineEdit *powerp2_edit;
        QPushButton *powerp_get;
        QPushButton *powerp_set;
        QGroupBox *jogp_box;
        QGridLayout *jogp_layout;
        QGroupBox *jogp_mode;
        QVBoxLayout *jogp_model;
        QRadioButton *jogp_mode1;
        QRadioButton *jogp_mode2;
        QVBoxLayout *jogp_stopmodel;
        QGroupBox *jogp_stopmode;
        QRadioButton *jogp_stopmode1;
        QRadioButton *jogp_stopmode2;
        QLabel *jogp_vell;
        QLineEdit *jogp_vele;
        QLabel *jogp_accl;
        QLineEdit *jogp_acce;
        QLabel *jogp_stepl;
        QLineEdit *jogp_stepe;
        QPushButton *jogp_set;
        QPushButton *jogp_get;
        QDialog *error_window;

        void Setup(int index);
};

class HelpText : public QWidget{
public:
    QPlainTextEdit *text;
    
    void Setup(){
        this->resize(600,300);
        std::string help_text;
        help_text.append("All input units are in encoder counts or microsteps, depends on controller and motor device. Refer to given device manual for conversion to real world units\n\n");
        help_text.append("Acceleration profiles\nAcceleration profiles are numbered form 0 to 18. 0 means trapezoidal, 1-18 S-shaped acceleration/decceleration curve. For closer info see documentation at page 63.\n\n");
        text = new QPlainTextEdit(help_text.c_str(),this);
        text->setReadOnly(true);
        text->setGeometry(5,5,590,390);
        this->show();
    }
    
};

class GUI: public QMainWindow{
     Q_OBJECT
    
    public slots:
        void openDevOptions();
        void openMoveOptions();
        void openHelp();
        void openDoc();
  
    public:
        int actual_channel;
        QMenuBar *menuBar;
        QMenu *control_menu;
        QMenu *device_menu;
        QMenu *help_menu;
        QAction **device_switch_actions;
        QAction *move_opt_action;
        QAction *device_opt_action;
        QAction *help_action;
        QAction *doc_action;
        QLabel *dev_name;
        QLabel *dev_ser;
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
        HelpText *helpw;
        QTimer *update_timer;
        QDialog *error_window;
        
        void Setup();
        
};




#endif	/* GUI_HPP */

