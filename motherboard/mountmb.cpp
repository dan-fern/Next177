/* mountmb.cpp contains main callouts for motherboard mounting calculator.
 *
 * loadData() does some basic error checking, loads a .csv file, and populates appropriate fields.
 *
 * saveData() checks for duplicate data, updates the saveTable, and writes the saveTable contents
 * to a .csv file.
 *
 * clearData() clears all fields and resets the dataLoaded boolean.
 *
 * calculateData() checks that all required fields are populated and then calculates FPA Angle
 * and Optical Centerline.  The calculated values are then checked against the design spec and
 * color-coded accordingly.
 *
 * getScreenShot() takes a screenshot of the current window and saves to a desired directory.
 *
 * showNotepad(), showCalculations(), showBuildData(), showTutorial(), showAbout() all reference
 * functions in the ViewBuildData class.
 *
 * initializeTables() sets up the save tables structures for load/save.
 *
 * updateSaveTable() updates the save tables before writing to .csv.
 *
 * fileExists() and checkText() are error checking functions.
*/

#include "mountmb.h"
#include "ui_mountmb.h"

MountMB::MountMB(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MountMB)
{
    ui->setupUi(this);
    // set up local variables from XML in .ui file
    inputControl = MountMB::findChild<QLineEdit *>("lineEditControl");
    inputSerial = MountMB::findChild<QLineEdit *>("lineEditSerial");
    inputSCA1y = MountMB::findChild<QLineEdit *>("lineEditSCA1y");
    inputSCA1z = MountMB::findChild<QLineEdit *>("lineEditSCA1z");
    inputSCA2y = MountMB::findChild<QLineEdit *>("lineEditSCA2y");
    inputSCA2z = MountMB::findChild<QLineEdit *>("lineEditSCA2z");
    outputAngle = MountMB::findChild<QLabel *>("labelOutputAngle");
    outputCenter = MountMB::findChild<QLabel *>("labelOutputCenter");
    // dataLoaded is a boolean which will tell whether data has been loaded
    dataLoaded = false;
    // this is the saving table template path, then tables are initialized
    pathTemplate = new QString("control/saveTemplate.csv");
    initializeTables( pathTemplate );
    controlInputDialog = new QInputDialog();
    kickBox = new QMessageBox();
    viewBuildData = new ViewBuildData();
}

void MountMB::loadData() {
    bool ok;
    // reset saving tables
    initializeTables( pathTemplate );
    controlInputDialog->setOptions(QInputDialog::NoButtons);
    QString inputText = controlInputDialog->getText(this, "Load Data", "Wand or input Control Number:",
                                      QLineEdit::Normal, inputControl->text(), &ok);
    if (!ok)
        return;
    QString loadText = checkText( inputText );
    if (!goodText)
        return;
    QString fileName = "control/" + loadText + ".csv";
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        kickBox->information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    QMap <QString, QString> data;
    int counter = 1;
    // while loop to populate tables with values from .csv
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        data.insert(line.split(',').first(), line.split(',').last().trimmed());
        saveTable[counter++] = line.split(',').last().trimmed();
    }
    file.close();
    if(data.isEmpty()) {
        kickBox->information(this, tr("No data in file"),
                tr("The file you are attempting to open contains no data."));
    } else {
        dataLoaded = true;
        // populate fields in calculator with table data
        inputControl->setText(data.value(saveTemplate[1]));
        inputSerial->setText(data.value(saveTemplate[2]));
        inputSCA1y->setText(QString::number(data.value(saveTemplate[3]).toDouble(), 'f', 4));
        inputSCA1z->setText(QString::number(data.value(saveTemplate[4]).toDouble(), 'f', 4));
        inputSCA2y->setText(QString::number(data.value(saveTemplate[5]).toDouble(), 'f', 4));
        inputSCA2z->setText(QString::number(data.value(saveTemplate[6]).toDouble(), 'f', 4));
        // once data loaded, calculate end values and lock control number and serial number fields.
        calculateData( );
        inputControl->setEnabled(false);
        inputSerial->setEnabled(false);
    }
}

void MountMB::saveData() {
    bool ok;
    if ( inputSerial->text().isEmpty() ) {
        kickBox->warning(this, tr("Save Error"), tr("No dewar serial number input."));
        return;
    // next 2 else ifs standardize serial number input
    } else if ( inputSerial->text().length() == 2 ) {
        QString str = "0" + inputSerial->text();
        inputSerial->setText( str );
    } else if ( inputSerial->text().length() == 1 ) {
        QString str = "00" + inputSerial->text();
        inputSerial->setText( str );
    }
    controlInputDialog->setOptions(QInputDialog::NoButtons);
    QString inputText = controlInputDialog->getText(this, "Save Data", "Wand or input Control Number:",
                                      QLineEdit::Normal, inputControl->text(), &ok);
    if (!ok)
        return;
    QString saveText = checkText( inputText );
    if (!goodText)
        return;
    inputControl->setText(saveText);
    QString fileName = "control/" + saveText + ".csv";
    // check for duplicate files and if so, should the file be overwritten
    if (!dataLoaded && fileExists(fileName)) {
        kickBox->warning(this, tr("Saved Data Detected"),
                            tr("Save data for C%1 detected but not loaded.\n"
                               "To avoid overwriting production history, "
                               "load control data before saving.").arg(saveText));
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Overwrite Data",
                            tr("Are you sure you would like to overwrite %1?").arg(saveText),
                                    QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }
    // update all tables from current calculator fields for writing to .csv
    updateSaveTable( );
    QFile file(fileName);
    QMap <QString, QString> data;
    if(file.open(QFile::WriteOnly|QFile::Truncate)) {
        QTextStream stream(&file);
        int rowCount = saveTable.size() - 1;
        // populate file stream with table data and close
        for (int i = 1; i <= rowCount; i++) {
            data.insert(saveTemplate[i], saveTable[i]);
            stream << saveTemplate[i] << ",\t" << saveTable[i] << endl;
        }
        file.close();
    } else {
        kickBox->information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    if(data.isEmpty()) {
        kickBox->information(this, tr("No data in file"),
                tr("The file you are attempting to save contains no data."));
    }
}

void MountMB::clearData() {
    dataLoaded = false;
    // error if no fields populated
    if (inputSCA1y->text().isEmpty() && inputSCA1z->text().isEmpty()
            && inputSCA2y->text().isEmpty() && inputSCA2z->text().isEmpty()) {
        kickBox->warning(this, tr("Clear Error!!"), tr("No data to clear."));
        return;
    } else {
        // clear text, clear format, reenable all fields, and reset save tables
        inputSCA1y->clear();
        inputSCA1z->clear();
        inputSCA2y->clear();
        inputSCA2z->clear();
        outputAngle->clear();
        outputAngle->setStyleSheet("");
        outputCenter->clear();
        outputCenter->setStyleSheet("");
    }
    inputControl->setEnabled(true);
    inputSerial->setEnabled(true);
    initializeTables( pathTemplate );
}

void MountMB::calculateData() {
    // check for usable data before calculating
    if (inputSCA1y->text().isEmpty() || inputSCA1z->text().isEmpty()
            || inputSCA2y->text().isEmpty() || inputSCA2z->text().isEmpty())
        kickBox->warning(this, tr("Calculate Error!!"), tr("No data to calculate."));
    else if (!inputSCA1y->text().toDouble() || !inputSCA1z->text().toDouble()
            || !inputSCA2y->text().toDouble() || !inputSCA2z->text().toDouble())
        kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
    else {
        // create local variables for calculating
        double angle;
        double center;
        double y1 = inputSCA1y->text().toDouble();
        double z1 = inputSCA1z->text().toDouble();
        double y2 = inputSCA2y->text().toDouble();
        double z2 = inputSCA2z->text().toDouble();

        angle = atan( (z2 - z1) / (y2 - y1) ) * ( 180 / M_PI );

        center = ( (z1 - z2) / 2 ) + z2;

        QString angleShow = QString::number(angle, 'f', 4);
        outputAngle->setText(angleShow);
        QString centerShow = QString::number(center, 'f', 4);
        outputCenter->setText(centerShow);
        // once calculated, populate output objects and color-code according to spec
        if (angle > 11.53 || angle < 10.93)
            outputAngle->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputAngle->setStyleSheet("QLabel { background-color : green; color : black; }");
        if (center > 0.015 || center < 0.011)
            outputCenter->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputCenter->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
}

void MountMB::getScreenShot() {
    // grab current window and save to desired directory as .png, .xpm, .jpg
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Screen Shot"), "",
                                        tr("Images (*.png *.xpm *.jpg);;All Files (*)"));
    if(fileName.isEmpty())
        return;
    else {
        QPixmap screenShot = QPixmap::grabWidget(ui->centralWidget);
        screenShot.save(fileName);
    }
}

void MountMB::showNotepad() {
    viewBuildData->showNotePad();
}

void MountMB::showCalculations() {
    viewBuildData->showLink( QString("calcs") );
}

void MountMB::showBuildData() {
    viewBuildData->showTable(saveTemplate, saveTable);
    viewBuildData->show();
}

void MountMB::showTutorial() {
    viewBuildData->showLink( QString("tutorial") );
}

void MountMB::showAbout() {
    QString exeName = "MotherboardMount.exe\n";
    viewBuildData->showAbout( exeName );
}

void MountMB::initializeTables( QString* path ) {
    // reset all tables at .exe launch or during clearData(), etc.
    saveTemplate.clear();
    saveTable.clear();
    QFile templat(*path);
    if(!templat.open(QIODevice::ReadOnly)) {
        kickBox->information(this, tr("Unable to open file"), templat.errorString());
        return;
    }
    while (!templat.atEnd()) {
        QByteArray line = templat.readLine();
        saveTemplate << line.split(',').first().trimmed();
        saveTable << line.split(',').last().trimmed();
    }
    // bandaid to line up indices :(
    saveTemplate.prepend("@@@");
    saveTable.prepend("@@@");
    templat.close();
}

void MountMB::updateSaveTable( ) {
    // get all current data from calulator fields and insert in saveTable array
    saveTable[1] = inputControl->text();
    saveTable[2] = inputSerial->text();
    saveTable[3] = inputSCA1y->text();
    saveTable[4] = inputSCA1z->text();
    saveTable[5] = inputSCA2y->text();
    saveTable[6] = inputSCA2z->text();
    saveTable[7] = outputAngle->text();
    saveTable[8] = outputCenter->text();
    // asterisks are used to tell saveData() in later calculators whether file is new or not
    // default in saveTemplate is "$$$", which is overwritten to "***", etc.
    saveTable[9] = "***";
    saveTemplate[9] = "***";
}

bool MountMB::fileExists( QString path ) {
    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile())
        return true;
    else
        return false;
}

QString MountMB::checkText( QString text ) {
    // checks control number input for format, edits out leading 'C' or trailing space
    goodText = false;
    if ( text.isEmpty() || text.length()>12 || text.length()<10 ) {
        kickBox->warning(this, tr("Input Error"),
                             tr("Enter 10-digit Control with or without leading\"C\""));
        return text;
    }
    if(text.at(0) == 'C' || text.at(0) == 'c')
        text.remove(text.at(0));
    if(text.at(text.length()-1) == ' ')
        text.remove(text.at(text.length()-1));
    if(text.toDouble() == 0 || text.length() != 10) {
        kickBox->warning(this, tr("Input Error"),
                             tr("Control Number must be 10 digits. %1").arg(text));
        return text;
    }
    goodText = true;
    return text;
}

MountMB::~MountMB()
{
    delete inputControl;
    delete inputSerial;
    delete inputSCA1y;
    delete inputSCA1z;
    delete inputSCA2y;
    delete inputSCA2z;
    delete outputAngle;
    delete outputCenter;
    delete pathTemplate;
    delete controlInputDialog;
    delete kickBox;
    delete viewBuildData;
    delete ui;
}
