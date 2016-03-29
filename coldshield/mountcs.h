#ifndef MOUNTCS_H
#define MOUNTCS_H

#include <QMainWindow>
#include <QtGui>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QList>
#include <QMap>
#include <cmath>
#include <algorithm>
#include <iostream>

class QLabel;
class QLineEdit;

namespace Ui {
class MountCS;
}

class MountCS : public QMainWindow
{
    Q_OBJECT

public:
    explicit MountCS(QWidget *parent = 0);
    ~MountCS();

public slots:
    void loadData();
    void saveData();
    void clearData();
    void calculateData();

private:
    Ui::MountCS *ui;
    QLineEdit *inputControl;
    QLineEdit *inputSerial;
    QLineEdit *inputCS;
    QLineEdit *inputFPA;
    QLineEdit *inputCF;
    QLineEdit *inputBL;
    QLineEdit *inputPlateau1;
    QLineEdit *inputPlateau2;
    QLineEdit *inputPlateau3;
    QLineEdit *inputPlateau4;
    QLabel *outputHeight;
    QLabel *outputParallel;
    bool plateauCalc;
    bool dataLoaded;
    QMap <int, QString> saveTemplate;
    QMap <int, QString> saveTable;
    QMap <int, QString> updateSaveTable( );
    bool fileExists( QString );
};

#endif // MOUNTCS_H
