#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plcmodbus.h"
#include "joyhid.h"

#include <QThread>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), joystck(nullptr)
{
    ui->setupUi(this);

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(70); // 10 seconds

    /*Joystick*/
    QThread *joy_thread = new QThread;
    joystck = new JoyHID;
    joystck -> moveToThread(joy_thread);
    connect(joy_thread, &QThread::started, joystck, &JoyHID::init_machine);
    connect(joystck, &JoyHID::updateUI, this, &MainWindow::uiSlot);

    joy_thread->start();

    /*Modbus*/
    plc = new PlcModbus;
    connect(joystck, &JoyHID::updateUI, plc, &PlcModbus::updateHID);
    connect(timer, &QTimer::timeout, plc, &PlcModbus::plcReadWrite);

    timer->start();
}

MainWindow::~MainWindow()
{
    delete joystck;
    delete plc;
    delete ui;
}

void MainWindow::uiSlot(HID_JOYSTK_Info_TypeDef *dev)
{
    ui->lineEdit->setText(QString::number(dev->X));
    ui->lineEdit_2->setText(QString::number(dev->Y));
    ui->lineEdit_3->setText(QString::number(dev->Z));
    ui->lineEdit_4->setText(QString::number(dev->X_low));
    ui->lineEdit_5->setText(QString::number(dev->Y_low));
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if (index==3)
        MainWindow::close();
}

