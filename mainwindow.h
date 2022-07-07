#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include "typedefs.h"

class PlcModbus;
class JoyHID;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void uiSlot(HID_JOYSTK_Info_TypeDef *dev);

private slots:
    void on_tabWidget_tabBarClicked(int index);

private:
    Ui::MainWindow *ui;
    PlcModbus *plc = nullptr;
    JoyHID *joystck = nullptr;
};
#endif // MAINWINDOW_H
