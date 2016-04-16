#ifndef GUI_HPP
#define	GUI_HPP

#include <qapplication.h>
#include <qpushbutton.h>
#include <QApplication>
#include <QWidget>

int run_gui(int argc, char** argv){
    QApplication a( argc, argv );
    QWidget *parent = new QWidget();
    parent->resize(400, 400);
    parent->show();
    QPushButton hello( "Hello world!", parent);
    hello.move(100,100);
    hello.resize( 100, 30 );
    hello.show();
    return a.exec();
}


#endif	/* GUI_HPP */

