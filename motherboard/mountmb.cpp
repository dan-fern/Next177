#include "mountmb.h"
#include "ui_mountmb.h"

MountMB::MountMB(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MountMB)
{
    ui->setupUi(this);
    inputControl = MountMB::findChild<QLineEdit *>("lineEditControl");
    inputSerial = MountMB::findChild<QLineEdit *>("lineEditSerial");
    inputSCA1y = MountMB::findChild<QLineEdit *>("lineEditSCA1y");
    inputSCA1z = MountMB::findChild<QLineEdit *>("lineEditSCA1z");
    inputSCA2y = MountMB::findChild<QLineEdit *>("lineEditSCA2y");
    inputSCA2z = MountMB::findChild<QLineEdit *>("lineEditSCA2z");
    outputAngle = MountMB::findChild<QLabel *>("labelOutputAngle");
    outputCenter = MountMB::findChild<QLabel *>("labelOutputCenter");
    dataLoaded = false;

    QFile templat("control/templateMB.csv");
    if(!templat.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 templat.errorString());
        return;
    }
    int i = 1;
    while (!templat.atEnd()) {
        QByteArray line = templat.readLine();
        saveTemplate.insert(i, line.split(',').first().trimmed());
        saveTable.insert(i, "0.0");
        i++;
    }
    saveTemplate.insert(i, "***");
    saveTable.insert(i, "***");
    templat.close();
}

void MountMB::loadData() {
    bool ok;
    QInputDialog* inputD = new QInputDialog();
    inputD->setOptions(QInputDialog::NoButtons);
    QString text = inputD->getText(this, "Load Data", "Wand or input Control Number:",
                                      QLineEdit::Normal, inputControl->text(), &ok);
    if (!ok) {
        delete inputD;
        return;
    } else if ( ok && (text.isEmpty() || text.length()>12 || text.length()<10) ) {
        QMessageBox::warning(this, tr("Input Error"),
                             tr("Enter 10-digit Control with or without leading\"C\""));
		delete inputD;
		return;
    }
    if(text.at(0) == 'C' || text.at(0) == 'c')
        text.remove(text.at(0));
    if(text.at(text.length()-1) == ' ')
        text.remove(text.at(text.length()-1));
    if(text.toDouble() == 0 || text.length() != 10) {
        QMessageBox::warning(this, tr("Input Error"), tr("Control Number must be 10 digits. %1").arg(text));
        delete inputD;
        return;
    }
    QString fileName = "control/" + text.append(".csv");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        delete inputD;
        return;
    }
    QMap <QString, QString> data;
    int counter = 1;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        data.insert(line.split(',').first(), line.split(',').last().trimmed());
        saveTable[counter++] = line.split(',').last().trimmed();
    }
    file.close();
    if(data.isEmpty()) {
        QMessageBox::information(this, tr("No data in file"),
                tr("The file you are attempting to open contains no data."));
    } else {
        dataLoaded = true;
        inputControl->setText(data.value(saveTemplate.value(1)));
        inputSerial->setText(data.value(saveTemplate.value(2)));
        inputSCA1y->setText(QString::number(data.value(saveTemplate.value(3)).toDouble(), 'f', 4));
        inputSCA1z->setText(QString::number(data.value(saveTemplate.value(4)).toDouble(), 'f', 4));
        inputSCA2y->setText(QString::number(data.value(saveTemplate.value(5)).toDouble(), 'f', 4));
        inputSCA2z->setText(QString::number(data.value(saveTemplate.value(6)).toDouble(), 'f', 4));
        calculateData( );
        inputControl->setEnabled(false);
        inputSerial->setEnabled(false);
    }
    delete inputD;
}

void MountMB::saveData() {
    bool ok;
    if ( inputSerial->text().isEmpty() ) {
        QMessageBox::warning(this, tr("Save Error"), tr("No dewar serial number input."));
        return;
    } else if ( inputSerial->text().length() == 2 ) {
        QString str = "0" + inputSerial->text();
        inputSerial->setText( str );
    } else if ( inputSerial->text().length() == 1 ) {
        QString str = "00" + inputSerial->text();
        inputSerial->setText( str );
    }
    QInputDialog* inputD = new QInputDialog();
    inputD->setOptions(QInputDialog::NoButtons);
    QString text = inputD->getText(this, "Save Data", "Wand or input Control Number:",
                                      QLineEdit::Normal, inputControl->text(), &ok);
    if (!ok) {
        delete inputD;
        return;
    } else if ( ok && (text.isEmpty() || text.length()>12 || text.length()<10) ) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Enter 10-digit Control with or without leading\"C\""));
		delete inputD;
		return;
    }
    if(text.at(0) == 'C' || text.at(0) == 'c')
        text.remove(text.at(0));
    if(text.at(text.length()-1) == ' ')
        text.remove(text.at(text.length()-1));
    if(text.toDouble() == 0 || text.length() != 10) {
        QMessageBox::warning(this, tr("Input Error"), tr("Control Number must be 10 digits. %1").arg(text));
        delete inputD;
        return;
    }
    QString fileName = "control/" + text.append(".csv");
    if (!dataLoaded && fileExists(fileName)) {
        QMessageBox::warning(this, tr("Saved Data Detected"),
                            tr("Save data for C%1 detected but not loaded.\n"
                               "To avoid overwriting production history, "
                               "load control data before saving.").arg(text));
        delete inputD;
        return;
    }
    saveTable = updateSaveTable( );
    QFile file(fileName);
    QMap <QString, QString> data;
    if(file.open(QFile::WriteOnly|QFile::Truncate)) {
        QTextStream stream(&file);
        int rowCount = saveTable.size();
        for (int i = 1; i <= rowCount; i++) {
            data.insert(saveTemplate.value(i), saveTable.value(i));
            stream << saveTemplate.value(i) << ",\t" << saveTable.value(i) << endl;
        }
        file.close();
    } else {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        delete inputD;
        return;
    }
    if(data.isEmpty()) {
        QMessageBox::information(this, tr("No data in file"),
                tr("The file you are attempting to save contains no data."));
    }
    delete inputD;
}

void MountMB::clearData() {
    dataLoaded = false;
    if (inputSCA1y->text().isEmpty() && inputSCA1z->text().isEmpty()
            && inputSCA2y->text().isEmpty() && inputSCA2z->text().isEmpty()) {
        QMessageBox::warning(this, tr("Clear Error!!"), tr("No data to clear."));
        return;
    } else {
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
}

void MountMB::calculateData() {
    if (inputSCA1y->text().isEmpty() || inputSCA1z->text().isEmpty()
            || inputSCA2y->text().isEmpty() || inputSCA2z->text().isEmpty())
        QMessageBox::warning(this, tr("Calculate Error!!"), tr("No data to calculate."));
    else if (!inputSCA1y->text().toDouble() || !inputSCA1z->text().toDouble()
            || !inputSCA2y->text().toDouble() || !inputSCA2z->text().toDouble())
        QMessageBox::warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
    else {
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

QMap<int, QString> MountMB::updateSaveTable( ) {
    saveTable[1] = inputControl->text();
    saveTable[2] = inputSerial->text();
    saveTable[3] = inputSCA1y->text();
    saveTable[4] = inputSCA1z->text();
    saveTable[5] = inputSCA2y->text();
    saveTable[6] = inputSCA2z->text();
    saveTable[7] = outputAngle->text();
    saveTable[8] = outputCenter->text();

    return saveTable;
}

bool MountMB::fileExists( QString path ) {
    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile())
        return true;
    else
        return false;
}

MountMB::~MountMB()
{
    delete ui;
}
