#include <QStateMachine>
#include <QApplication>
#include <QDebug>
#include <QThread>

#include "joyhid.h"

JoyHID::JoyHID(QObject *parent)
    : QObject{parent}
{
    hid_init();
    handle = hid_open(0x231d, 0x126, NULL); // Open device, use VID and PID
    if (!handle) {
        qInfo()<< "ERROR. USB not connected";
    } else {
        qInfo()<< "OK. USB connected";
    }
    if (hid_set_nonblocking(handle, 1) < 0) { // Set hid_read() function nonblocking.
        qInfo()<< "ERROR. Non-blocking not set";
    } else {
        qInfo()<< "OK. Non-blocking set";
    }
}

JoyHID::~JoyHID()
{
    hid_exit(); //
}

void JoyHID::joyStart()
{
    QThread::usleep(20); // Thread synchro delay
    emit toUpdate();
}

void JoyHID::joyHIDUpdate()
{
    unsigned char buf[64];

    memset(buf, 0x00, sizeof(buf)); // Set command buf.
    buf[0] = 0x01;
    buf[1] = 0x81;

    if ((hid_enumerate(0x231d, 0x126) != NULL) || (wcscmp(hid_error(handle), L"Success"))) {
        if (!handle) {
            handle = hid_open(0x231d, 0x126, NULL); // Open (restart) device, use VID and PID
        } else {
            hid_read(handle, buf, 64); // Get report
            if((((uint16_t)buf[2] << 8) | buf[1])!=129) {
                joyInfo.X = ((uint16_t)buf[2] << 8) | buf[1];
                joyInfo.Y = ((uint16_t)buf[4] << 8) | buf[3];
                joyInfo.Z = ((uint16_t)buf[10] << 8) | buf[9];
                joyInfo.X_low = ((uint16_t)buf[6] << 8) | buf[5];
                joyInfo.Y_low = ((uint16_t)buf[8] << 8) | buf[7];
                joyInfo.buttons[0] = 1;
                joyInfo.buttons[1] = 0;
                joyInfo.buttons[2] = 1;
                joyInfo.buttons[3] = 0;
                joyInfo.buttons[4] = 1;
            }
            emit updateUI(&joyInfo); // Signal for updating joystick values
        }
    } else {
        if (handle) {
            hid_close(handle); // Close if error detected
            handle = NULL;
        }
    }
    emit toStart_All(); // Next state
}

void JoyHID::startMachine()
{
    qInfo()<< "Joy machine running";
}

void JoyHID::init_machine()
{
    QStateMachine *joy_machine = new QStateMachine();
    QState *joy_start = new QState();
    QState *joy_update = new QState();

    joy_start->addTransition(this, &JoyHID::toUpdate, joy_update);
    joy_update->addTransition(this, &JoyHID::toStart_All, joy_start);

    joy_machine->addState(joy_start);
    joy_machine->addState(joy_update);

    joy_machine->setInitialState(joy_start);

    connect(joy_start, &QState::entered, this, &JoyHID::joyStart);
    connect(joy_update, &QState::entered, this, &JoyHID::joyHIDUpdate);
    connect(joy_machine, &QStateMachine::started, this, &JoyHID::startMachine);

    joy_machine->start();
}
