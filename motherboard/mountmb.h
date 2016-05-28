#ifndef MOUNTMB_H
#define MOUNTMB_H

#include <QMainWindow>
#include <QtGui>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QDesktopServices>
#include <QUrl>
#include <cmath>
#include <algorithm>
#include <iostream>

#include <viewbuilddata.h>

class QLabel;
class QLineEdit;
class QTextEdit;

namespace Ui {
class MountMB;
}

class MountMB : public QMainWindow
{
    Q_OBJECT

public:
    explicit MountMB(QWidget *parent = 0);
    //QMap <QString, QString> buildData;
    ~MountMB();

public slots:
    // in Qt, these are the callouts which link buttons, actions, etc. from UI (XML) to C++ code
    void loadData();
    void saveData();
    void clearData();
    void calculateData();
    void getScreenShot();
    void showNotepad();
    void showCalculations();
    void showBuildData();
    void showAbout();

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
    bool goodText;
    QMessageBox *kickBox;
    QInputDialog *controlInputDialog;
    QString *pathTemplate;
    QList <QString> saveTemplate;
    QList <QString> saveTable;
    void initializeTables( QString* );
    void updateSaveTable( );
    bool fileExists( QString );
    QString checkText( QString );
    ViewBuildData *viewBuildData;
};

#endif // MOUNTMB_H
