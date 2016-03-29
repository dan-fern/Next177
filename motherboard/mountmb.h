#ifndef MOUNTMB_H
#define MOUNTMB_H

#include <QMainWindow>
#include <QtGui>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMap>
#include <cmath>
#include <algorithm>
#include <iostream>

class QLabel;
class QLineEdit;

namespace Ui {
class MountMB;
}

class MountMB : public QMainWindow
{
    Q_OBJECT

public:
    explicit MountMB(QWidget *parent = 0);
    ~MountMB();

public slots:
    void loadData();
    void saveData();
    void clearData();
    void calculateData();

private:
    Ui::MountMB *ui;
    QLineEdit *inputControl;
    QLineEdit *inputSerial;
    QLineEdit *inputSCA1y;
    QLineEdit *inputSCA1z;
    QLineEdit *inputSCA2y;
    QLineEdit *inputSCA2z;
    QLabel *outputAngle;
    QLabel *outputCenter;
    bool dataLoaded;
    QMap <int, QString> saveTemplate;
    QMap <int, QString> saveTable;
    QMap <int, QString> updateSaveTable( );
    bool fileExists( QString );
};

#endif // MOUNTMB_H
