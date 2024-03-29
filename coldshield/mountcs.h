#ifndef MOUNTCS_H
#define MOUNTCS_H

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
#include <proteuslookup.h>

class QLabel;
class QLineEdit;
class QTextEdit;

namespace Ui {
class MountCS;
}

class MountCS : public QMainWindow
{
    Q_OBJECT

public:
    explicit MountCS(QWidget *parent = 0); 
    //QMap <QString, QString> buildData;
    ~MountCS();

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
    void showTutorial();
    void showAbout();
    void checkProteusData1061( );

private:
    Ui::MountCS *ui;
    QLineEdit *inputControl;
    QLineEdit *inputSerial;
    QLineEdit *inputCS;
    QLineEdit *inputFPA;
    QLineEdit *inputCF;
    QComboBox *inputBL;
    QLineEdit *inputPlateau1;
    QLineEdit *inputPlateau2;
    QLineEdit *inputPlateau3;
    QLineEdit *inputPlateau4;
    QLabel *outputHeight;
    QLabel *outputParallel;
    bool plateauCalc;
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
    ProteusLookup *proteus;
    //QString *rawProteusText;
};

#endif // MOUNTCS_H
