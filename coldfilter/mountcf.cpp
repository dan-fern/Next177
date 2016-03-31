#include "mountcf.h"
#include "ui_mountcf.h"

MountCF::MountCF(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MountCF)
{
    ui->setupUi(this);
    inputControl = MountCF::findChild<QLineEdit *>("lineEditControl");
    inputSerial = MountCF::findChild<QLineEdit *>("lineEditSerial");
    inputCF1 = MountCF::findChild<QLineEdit *>("lineEditCF1");
    inputCF2 = MountCF::findChild<QLineEdit *>("lineEditCF2");
    inputCS = MountCF::findChild<QLineEdit *>("lineEditCS");
    inputFPA1 = MountCF::findChild<QLineEdit *>("lineEditFPA1");
    inputFPA2 = MountCF::findChild<QLineEdit *>("lineEditFPA2");
    inputFiducial1 = MountCF::findChild<QLineEdit *>("lineEditFid1");
    inputFiducial2 = MountCF::findChild<QLineEdit *>("lineEditFid2");
    inputFiducial3 = MountCF::findChild<QLineEdit *>("lineEditFid3");
    outputBond = MountCF::findChild<QLabel *>("labelOutputBL");
    outputBalls = MountCF::findChild<QLabel *>("labelOutputBH");
    outputHeight1 = MountCF::findChild<QLabel *>("labelOutputHeight1");
    outputHeight2 = MountCF::findChild<QLabel *>("labelOutputHeight2");
    outputParallel = MountCF::findChild<QLabel *>("labelOutputParallel");
    calc1 = false;
    calc2 = false;
    dataLoaded = false;
    pathTemplate = new QString("control/templateCF.csv");
    initializeTables( pathTemplate );
    controlInputDialog = new QInputDialog();
    kickBox = new QMessageBox();
}

void MountCF::loadData() {
    bool ok;
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
        inputControl->setText(data.value(saveTemplate[1]));
        inputSerial->setText(data.value(saveTemplate[2]));
        if (saveTable.contains("$$$$$")) {
            //no previous calc data
            inputCS->setText(QString::number(data.value(saveTemplate[14]).toDouble(), 'f', 4));
            inputFPA1->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
            inputFPA2->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
        } else {
            //only calc1 data
            inputCF1->setText(QString::number(data.value(saveTemplate[21]).toDouble(), 'f', 4));
            inputCS->setText(QString::number(data.value(saveTemplate[22]).toDouble(), 'f', 4));
            inputFPA1->setText(QString::number(data.value(saveTemplate[23]).toDouble(), 'f', 4));
            calculateData1( );
            inputFPA2->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
            inputCS->setEnabled(false);
            inputFPA1->setEnabled(false);
        }
        if (data.contains("******")) {
            //calc1 and calc2 data both exist
            inputFiducial1->setText(QString::number(data.value(saveTemplate[28]).toDouble(), 'f', 4));
            inputFiducial2->setText(QString::number(data.value(saveTemplate[29]).toDouble(), 'f', 4));
            inputFiducial3->setText(QString::number(data.value(saveTemplate[30]).toDouble(), 'f', 4));
            inputCF2->setText(QString::number(data.value(saveTemplate[31]).toDouble(), 'f', 4));
            inputFPA2->setText(QString::number(data.value(saveTemplate[32]).toDouble(), 'f', 4));
            calculateData2( );
            inputFPA2->setEnabled(false);
        }
        inputControl->setEnabled(false);
        inputSerial->setEnabled(false);
    }
}

void MountCF::saveData() {
    bool ok;
    if ( inputSerial->text().isEmpty() ) {
        kickBox->warning(this, tr("Save Error"), tr("No dewar serial number input."));
        return;
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
    if (!dataLoaded && fileExists(fileName)) {
        if (!calc1){
            kickBox->warning(this, tr("Saved Data Detected"),
                                tr("Save data for C%1 detected but not loaded.\n"
                                   "To avoid overwriting production history, "
                                   "load control data before saving.").arg(saveText));
        } else {
            kickBox->warning(this, tr("Saved Data Detected"),
                                tr("Save data for C%1 partially loaded.\n"
                                   "Saving additional data will overwrite existing data.\n"
                                   "Verify all existing and additional data prior to saving.").arg(saveText));
        }
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Overwrite Data",
                            tr("Are you sure you would like to overwrite %1?").arg(saveText),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }
    updateSaveTable( calc1, calc2 );
    QFile file(fileName);
    QMap <QString, QString> data;
    if(file.open(QFile::WriteOnly|QFile::Truncate)) {
        QTextStream stream(&file);
        int rowCount = saveTable.size() - 1;
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

void MountCF::clearData() {
    dataLoaded = false;
    if( inputFiducial1->text().isEmpty() && inputFiducial2->text().isEmpty()
                && inputFiducial3->text().isEmpty() && inputCS->text().isEmpty()
                && inputFPA1->text().isEmpty() && inputFPA2->text().isEmpty()
                && inputCF1->text().isEmpty() && inputCF2->text().isEmpty() ) {
        kickBox->warning(this, tr("Clear Error!!"), tr("No data to clear."));
        inputFiducial1->setEnabled(true);
        inputFiducial2->setEnabled(true);
        inputFiducial3->setEnabled(true);
        inputCF2->setEnabled(true);
        inputCS->setEnabled(true);
        inputFPA1->setEnabled(true);
        inputFPA2->setEnabled(true);\
        inputControl->setEnabled(true);
        inputSerial->setEnabled(true);
    } else if (calc2) {
        calc2 = false;
        inputFiducial1->clear();
        inputFiducial2->clear();
        inputFiducial3->clear();
        inputCF2->clear();
        inputFPA2->clear();
        inputFPA2->setEnabled(true);
        if (!fiducialCalc) {
            inputFiducial1->setEnabled(true);
            inputFiducial2->setEnabled(true);
            inputFiducial3->setEnabled(true);
        } else {
            inputCF2->setEnabled(true);
        }
        outputHeight2->clear();
        outputHeight2->setStyleSheet("");
        outputParallel->clear();
        outputParallel->setStyleSheet("");
    } else if (calc1) {
        calc1 = false;
        inputCF1->clear();
        inputCS->clear();
        inputCS->setEnabled(true);
        inputFPA1->clear();
        inputFPA1->setEnabled(true);
        outputBond->clear();
        outputBalls->clear();
        outputHeight1->clear();
        outputHeight1->setStyleSheet("");
        inputControl->setEnabled(true);
        inputSerial->setEnabled(true);
    } else {
        inputFiducial1->clear();
        inputFiducial2->clear();
        inputFiducial3->clear();
        inputCF2->clear();
        inputFPA2->clear();
        inputCF1->clear();
        inputCS->clear();
        inputFPA1->clear();
    }
    initializeTables( pathTemplate );
}

void MountCF::calculateData1() {
    if ( inputCF1->text().isEmpty() && inputCS->text().isEmpty()
         && inputFPA1->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("No input data given."));
        return;
    } else if( inputCF1->text().isEmpty() || inputCS->text().isEmpty()
               || inputFPA1->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
        return;
    } else if( !inputCF1->text().toDouble() || !inputCS->text().toDouble()
               || !inputFPA1->text().toDouble() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Input data must be numeric."));
        return;
    } else {
        calc1 = true;
        double bondline [] = { 0.0010, 0.0015, 0.0020, 0.0025, 0.0030 };
        std::vector<double> bond;
        bond.assign(bondline, bondline+5);
        std::vector<double> balls (bond.size());
        std::vector<double> sum (bond.size());
        double cf = inputCF1->text().toDouble();
        double cs = inputCS->text().toDouble();
        double fpa = inputFPA1->text().toDouble();
        int choice;

        for ( int i = 0; i < 5; i++ ) {
            sum[i] = abs(cf) + abs(cs) - abs(fpa) + abs(bond[i]);
            balls[i] =  sum[i] + abs(fpa);
            if (i == 0)
                choice = 0;
            else if ( abs(5.5973 - sum[i]) < abs(5.5973 - sum[choice]) )
                choice = i;
        }

        if( sum[choice] > 5.6013 || sum[choice] < 5.5933 ) {
            outputHeight1->setText(QString::number(sum[choice], 'f', 4));
            outputHeight1->setStyleSheet("QLabel { background-color : red; color : black; }");
            kickBox->warning(this, tr("ICD not met"),
                        tr("No possible bond line. Expected Height: %1").arg(sum[choice]));
            return;
        //} else if ( choice > 1 && abs(5.5973 - sum[choice]) <= bondline[0]/2 ) {
            //choice = choice - 1;
        //} else if ( choice > 1 && abs(5.5973 - sum[choice]) <= bondline[0] ) {
            //choice = choice - 2;
        }
        outputBond->setText(QString::number(bond[choice], 'f', 4));
        outputBalls->setText(QString::number(balls[choice], 'f', 4));
        outputHeight1->setText(QString::number(sum[choice], 'f', 4));
        outputHeight1->setStyleSheet("QLabel { background-color : green; color : black; }");

        //std::cout << bond[0] << " " << bond[1] << " " << bond[2] << " " << bond[4] << std::endl;
        //std::cout << sum[0] << " " << sum[1] << " " << sum[2] << " " << sum[4] << std::endl;
        //std::cout << balls[0] << " " << balls[1] << " " << balls[2] << " " << balls[4] << std::endl;
        //std::cout << bond[choice] << " " << sum[choice] << " " << balls[choice] << std::endl;

        //QMessageBox::warning(this, tr("BILLY'S HERE!!"), tr("Size: %1").arg(balls.size()));
        return;
    }

}

void MountCF::calculateData2() {
    if ( inputFiducial1->text().isEmpty() && inputFiducial2->text().isEmpty()
                && inputFiducial3->text().isEmpty() && inputCF2->text().isEmpty() ) {
            kickBox->warning(this, tr("Calculate Error!!"), tr("No input data given."));
            return;
    } else if ( inputFiducial1->text().isEmpty() || inputFiducial2->text().isEmpty()
                || inputFiducial3->text().isEmpty() ) {
            if ( inputCF2->text().isEmpty() || inputFPA2->text().isEmpty() ) {
                kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
                return;
            } else if ( !inputCF2->text().isEmpty() && inputCF2->text().toDouble()) {
                calc2 = true;
                fiducialCalc = false;
                inputFiducial1->setEnabled(false);
                inputFiducial2->setEnabled(false);
                inputFiducial3->setEnabled(false);
            } else {
                kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
                return;
            }
    } else {
        if( !inputFiducial1->text().toDouble() || !inputFiducial2->text().toDouble()
                || !inputFiducial3->text().toDouble() ) {
            kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
            return;
        }
        calc2 = true;
        fiducialCalc = true;
        inputCF2->setEnabled(false);
        outputHeight2->clear();
        outputHeight2->setStyleSheet("");
        double avg;
        double parallel;
        double fid1 = inputFiducial1->text().toDouble();
        double fid2 = inputFiducial2->text().toDouble();
        double fid3 = inputFiducial3->text().toDouble();
        QList<double> fiducials;
        fiducials << fid1 << fid2 << fid3;
        std::sort( fiducials.begin(), fiducials.end() );

        avg = ( abs(fid1) + abs(fid2) + abs(fid3) ) / fiducials.size();

        parallel = abs( fiducials.back() - fiducials.front() );

        inputCF2->setText(QString::number(avg, 'f', 4));
        outputParallel->setText(QString::number(parallel, 'f', 4));
        if ( parallel > 0.0030 )
            outputParallel->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputParallel->setStyleSheet("QLabel { background-color : green; color : black; }");
        }
    if ( inputFPA2->text().isEmpty() ) {
        return;
    } else if( inputCF2->text().isEmpty() || inputFPA2->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
        return;
    } else if( !inputCF2->text().toDouble() || !inputFPA2->text().toDouble() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
        return;
    } else {
        double sum;
        double cf = inputCF2->text().toDouble();
        double fpa = inputFPA2->text().toDouble();

        sum = abs(cf) - abs(fpa);

        outputHeight2->setText(QString::number(sum, 'f', 4));
        if(sum > 5.6013 || sum < 5.5933)
            outputHeight2->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputHeight2->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
}

void MountCF::initializeTables( QString* path ) {
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
    saveTemplate.prepend("@@@@@");
    saveTemplate.append("$$$$$$");
    saveTable.prepend("@@@@@");
    saveTable.append("$$$$$$");
    templat.close();
}

void MountCF::updateSaveTable( bool calculated1, bool calculated2 ) {
    saveTable[1] = inputControl->text();
    saveTable[2] = inputSerial->text();
    if (calculated1) {
        saveTable[21] = inputCF1->text();
        saveTable[22] = inputCS->text();
        saveTable[23] = inputFPA1->text();
        saveTable[24] = outputBond->text();
        saveTable[25] = outputBalls->text();
        saveTable[26] = outputHeight1->text();
        saveTable[27] = "*****";
        saveTemplate[27] = "*****";
    }
    if (calculated2) {
        saveTable[28] = inputFiducial1->text();
        saveTable[29] = inputFiducial2->text();
        saveTable[30] = inputFiducial3->text();
        saveTable[31] = inputCF2->text();
        saveTable[32] = inputFPA2->text();
        saveTable[33] = outputHeight2->text();
        saveTable[34] = outputParallel->text();
        saveTable[35] = "******";
        saveTemplate[35] = "******";
    }
}

bool MountCF::fileExists( QString path ) {
    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile())
        return true;
    else
        return false;
}

QString MountCF::checkText( QString text ) {
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

MountCF::~MountCF()
{
    delete pathTemplate;
    delete controlInputDialog;
    delete kickBox;
    delete ui;
}
