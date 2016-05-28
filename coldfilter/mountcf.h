#ifndef MOUNTCF_H
#define MOUNTCF_H

#include <QMainWindow>
#include <QtGui>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QMap>
#include <iostream>
#include <QDesktopServices>
#include <QUrl>
#include <cmath>
#include <algorithm>
#include <array>
#include <iostream>

#include <viewbuilddata.h>

class QLabel;
class QLineEdit;
class QTextEdit;

namespace Ui {
class MountCF;
}

class MountCF : public QMainWindow
{
    Q_OBJECT

public:
    explicit MountCF(QWidget *parent = 0);
    //QMap <QString, QString> buildData;
    ~MountCF();

public slots:
    void loadData();
    void saveData();
    void clearData();
    void calculateData1();
    void calculateData2();
    void getScreenShot();
    void showNotepad();
    void showCalculations();
    void showBuildData();
    void showAbout();

private:
    Ui::MountCF *ui;
    QLineEdit *inputControl;
    QLineEdit *inputSerial;
    QLineEdit *inputCF1;
    QLineEdit *inputCF2;
    QLineEdit *inputCS;
    QLineEdit *inputFPA1;
    QLineEdit *inputFPA2;
    QLineEdit *inputFiducial1;
    QLineEdit *inputFiducial2;
    QLineEdit *inputFiducial3;
    QLabel *outputBond;
    QLabel *outputBalls;
    QLabel *outputHeight1;
    QLabel *outputHeight2;
    QLabel *outputParallel;
    bool calc1;
    bool calc2;
    bool fiducialCalc;
    bool dataLoaded;
    bool goodText;
    QMessageBox *kickBox;
    QInputDialog *controlInputDialog;
    QString *pathTemplate;
    QList <QString> saveTemplate;
    QList <QString> saveTable;
    void initializeTables( QString* );
    void updateSaveTable( bool, bool );
    bool fileExists( QString );
    QString checkText( QString );
    ViewBuildData *viewBuildData;
};

#endif // MOUNTCF_H
