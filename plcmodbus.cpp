#include "plcmodbus.h"
#include "joyhid.h"

#include <QModbusRtuSerialMaster>
#include <QSerialPort>
#include <QDebug>

PlcModbus::PlcModbus(QObject *parent)
    : QObject{parent},
      plcDevice(nullptr)
{
    plcDevice = new QModbusRtuSerialMaster(this);
    connect(plcDevice, &QModbusClient::errorOccurred, this, [this](QModbusDevice::Error) {
        errorMessage(plcDevice->errorString());
        emit comError(plcDevice->errorString());
    });

    if(!plcDevice) {
        errorMessage(tr("Could not create Modbus master."));
        emit comError(tr("Could not create Modbus master."));
    } else {
        connect(plcDevice, &QModbusClient::stateChanged, this, &PlcModbus::onStateChanged);
    }

    portConnect();
}

PlcModbus::~PlcModbus()
{
    if (plcDevice)
        plcDevice->disconnectDevice();
}

/*Состояние порта*/
void PlcModbus::onStateChanged(int state)
{
    if(state == QModbusDevice::UnconnectedState) {
        qInfo() << "Port closed";
    } else if (state == QModbusDevice::ConnectedState) {
        qInfo() << "Port open";
    } else {
        return;
    }
}

/*Вывод ошибок*/
void PlcModbus::errorMessage(QString msg)
{
    qInfo() << "Error message: " << msg;
}

/*Подключение порта*/
void PlcModbus::portConnect()
{
    if(!plcDevice) return;
    if(plcDevice->state() != QModbusDevice::ConnectedState) {
        plcDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, QString("COM1"));
        plcDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
        plcDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud19200);
        plcDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
        plcDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
        plcDevice->setTimeout(100);
        plcDevice->setNumberOfRetries(0);
        if(!plcDevice->connectDevice()) {
            errorMessage("Connected failed: " + plcDevice->errorString());
            emit comError("Connected failed: " + plcDevice->errorString());
        } else {
            qInfo()<< "Port connected";
        }
    }
}
/*Запись и чтение*/
void PlcModbus::plcReadWrite()
{
    if (!plcDevice)
        return;

    QModbusDataUnit writeCoilsUnit = writeCoilsRequest();
    QModbusDataUnit writeHoldUnit = writeHoldRequest();

    writeHoldUnit.setValue(0, sentData.X);
    writeHoldUnit.setValue(1, sentData.Y);
    writeHoldUnit.setValue(2, sentData.Z);
    writeHoldUnit.setValue(3, sentData.X_low);
    writeHoldUnit.setValue(4, sentData.Y_low);

    writeCoilsUnit.setValue(0, sentData.buttons[0]);
    writeCoilsUnit.setValue(1, sentData.buttons[1]);
    writeCoilsUnit.setValue(2, sentData.buttons[2]);
    writeCoilsUnit.setValue(3, sentData.buttons[3]);
    writeCoilsUnit.setValue(4, sentData.buttons[4]);

    /*Holding Reg*/
    //    if (auto *reply = plcDevice->sendReadWriteRequest(readInputsRequest(), writeHoldUnit, 1)) {
    //        if (!reply->isFinished())
    //            connect(reply, &QModbusReply::finished, this, &PlcModbus::onReadReady);
    //        else
    //            delete reply; // broadcast replies return immediately
    //    } else {
    //        errorMessage(tr("Read error: ") + plcDevice->errorString());
    //    }

    //    /*Coils Reg*/
    //    if (auto *reply = plcDevice->sendReadWriteRequest(readDiscRequest(), writeCoilsUnit, 1)) {
    //        if (!reply->isFinished())
    //            connect(reply, &QModbusReply::finished, this, &PlcModbus::onReadReady);
    //        else
    //            delete reply; // broadcast replies return immediately
    //    } else {
    //        errorMessage(tr("Read error: ") + plcDevice->errorString());
    //    }
    /*Holding Reg*/

    if (auto *reply = plcDevice->sendWriteRequest(writeHoldUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    errorMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                 .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
                    emit modbusError(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                     .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
                } else if (reply->error() != QModbusDevice::NoError) {
                    errorMessage(tr("Write response error: %1 (code: 0x%2)").
                                 arg(reply->errorString()).arg(reply->error(), -1, 16));
                    emit modbusError(tr("Write response error: %1 (code: 0x%2)").
                                     arg(reply->errorString()).arg(reply->error(), -1, 16));
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        errorMessage(tr("Read error: ") + plcDevice->errorString());
        emit comError(tr("Read error: ") + plcDevice->errorString());
    }

//    if (auto *reply = plcDevice->sendWriteRequest(writeCoilsUnit, 1)) {
//        if (!reply->isFinished()) {
//            connect(reply, &QModbusReply::finished, this, [this, reply]() {
//                if (reply->error() == QModbusDevice::ProtocolError) {
//                    errorMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
//                                 .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
//                    emit modbusError(tr("Write response error: %1 (Mobus exception: 0x%2)")
//                                     .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
//                } else if (reply->error() != QModbusDevice::NoError) {
//                    errorMessage(tr("Write response error: %1 (code: 0x%2)").
//                                 arg(reply->errorString()).arg(reply->error(), -1, 16));
//                    emit modbusError(tr("Write response error: %1 (code: 0x%2)").
//                                     arg(reply->errorString()).arg(reply->error(), -1, 16));
//                }
//                reply->deleteLater();
//            });
//        } else {
//            // broadcast replies return immediately
//            reply->deleteLater();
//        }
//    } else {
//        errorMessage(tr("Read error: ") + plcDevice->errorString());
//        emit comError(tr("Read error: ") + plcDevice->errorString());
//    }

}

void PlcModbus::updateHID(HID_JOYSTK_Info_TypeDef *dev)
{
    sentData.X = dev->X;
    sentData.Y = dev->Y;
    sentData.Z = dev->Z;
    sentData.X_low = dev->X_low;
    sentData.Y_low = dev->Y_low;
    sentData.buttons[0] = dev->buttons[0];
    sentData.buttons[1] = dev->buttons[1];
    sentData.buttons[2] = dev->buttons[2];
    sentData.buttons[3] = dev->buttons[3];
    sentData.buttons[4] = dev->buttons[4];
}

void PlcModbus::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (int i = 0, total = int(unit.valueCount()); i < total; ++i) {
            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
                    .arg(QString::number(unit.value(i),
                                         unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        errorMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                     arg(reply->errorString()).
                     arg(reply->rawResult().exceptionCode(), -1, 16));
    } else {
        errorMessage(tr("Read response error: %1 (code: 0x%2)").
                     arg(reply->errorString()).
                     arg(reply->error(), -1, 16));
    }
    reply->deleteLater();
}

QModbusDataUnit PlcModbus::readDiscRequest() const
{
    return QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 5, 5);
}

QModbusDataUnit PlcModbus::readInputsRequest() const
{
    return QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 5);
}

QModbusDataUnit PlcModbus::writeCoilsRequest() const
{
    return QModbusDataUnit(QModbusDataUnit::Coils, 5, 5);
}

QModbusDataUnit PlcModbus::writeHoldRequest() const
{
    return QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 5);
}
