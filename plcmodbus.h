#ifndef PLCMODBUS_H
#define PLCMODBUS_H

#include <QObject>
#include <QModbusDataUnit>

#include "typedefs.h"

class QModbusClient;
class QStateMachine;

class PlcModbus : public QObject
{
    Q_OBJECT
public:
    explicit PlcModbus(QObject *parent = nullptr);
     ~PlcModbus();
signals:
    void modbusError(QString);
    void comError(QString);

private:
    QModbusDataUnit readDiscRequest() const;
    QModbusDataUnit readInputsRequest() const;

    QModbusDataUnit writeCoilsRequest() const;
    QModbusDataUnit writeHoldRequest() const;

public slots:
    void plcReadWrite();
    void updateHID(HID_JOYSTK_Info_TypeDef *dev);

private slots:
    void portConnect();
    void onStateChanged(int);

    void onReadReady();

    void errorMessage(QString msg);

private:
    QModbusClient *plcDevice = nullptr;
    HID_JOYSTK_Info_TypeDef sentData;
};

#endif // PLCMODBUS_H
