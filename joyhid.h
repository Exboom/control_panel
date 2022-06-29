#ifndef JOYHID_H
#define JOYHID_H

#include <QObject>

#include "hidapi.h"
#include "typedefs.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef HID_API_MAKE_VERSION
#define HID_API_MAKE_VERSION(mj, mn, p) (((mj) << 24) | ((mn) << 8) | (p))
#endif
#ifndef HID_API_VERSION
#define HID_API_VERSION HID_API_MAKE_VERSION(HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH)
#endif

#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_winapi.h>
#endif

class QStateMachine;
class QState;

#define MAX_STR 255

class JoyHID : public QObject
{
    Q_OBJECT

public:
    explicit JoyHID(QObject *parent = nullptr);
    HID_JOYSTK_Info_TypeDef joyInfo;

signals:
    void toStart_All();
    void toUpdate();
    void updateUI(HID_JOYSTK_Info_TypeDef *dev);
    void usbError(QString);

public slots:
    void init_machine();

private slots:
    void joyStart();
    void joyHIDUpdate();
    void startMachine();

private:
    hid_device *handle;
    QStateMachine *joy_machine = nullptr;
    QState *joy_start;
    QState *joy_update;
};

#endif // JOYHID_H
