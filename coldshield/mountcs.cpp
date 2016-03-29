#include "mountcs.h"
#include "ui_mountcs.h"

MountCS::MountCS(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MountCS)
{
    ui->setupUi(this);
    inputControl = MountCS::findChild<QLineEdit *>("lineEditControl");
    inputSerial = MountCS::findChild<QLineEdit *>("lineEditSerial");
    inputCS = MountCS::findChild<QLineEdit *>("lineEditCS");
    inputFPA = MountCS::findChild<QLineEdit *>("lineEditFPA");
    inputCF = MountCS::findChild<QLineEdit *>("lineEditCF");
    inputBL = MountCS::findChild<QLineEdit *>("lineEditBL");
    inputPlateau1 = MountCS::findChild<QLineEdit *>("lineEditPlateau1");
    inputPlateau2 = MountCS::findChild<QLineEdit *>("lineEditPlateau2");
    inputPlateau3 = MountCS::findChild<QLineEdit *>("lineEditPlateau3");
    inputPlateau4 = MountCS::findChild<QLineEdit *>("lineEditPlateau4");
    outputHeight = MountCS::findChild<QLabel *>("labelOutputHeight");
    outputParallel = MountCS::findChild<QLabel *>("labelOutputParallel");
    dataLoaded = false;

    QFile templat("control/templateCS.csv");
        if(!templat.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     templat.errorString());
            return;
        }
        int i = 1;
        while (!templat.atEnd()) {
            QByteArray line = templat.readLine();
            saveTemplate.insert(i, line.split(',').first().trimmed());
            saveTable.insert(i, line.split(',').last().trimmed());
            i++;
        }
        saveTemplate.insert(i, "****");
        saveTable.insert(i, "****");
        templat.close();
}

void MountCS::loadData() {
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
        QMessageBox::information(this, tr("Input Error"),
                            tr("Control Number must be 10 digits. %1").arg(text));
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
        if (!data.contains("****")) {
            inputCF->setText(QString::number(saveTable.value(15).toDouble(), 'f', 3));
            inputFPA->setText(QString::number(data.value(saveTemplate.value(8)).toDouble(), 'f', 4));
            inputBL->setText(QString::number(saveTable.value(17).toDouble(), 'f', 3));
        } else {
            inputPlateau1->setText(QString::number(data.value(saveTemplate.value(10)).toDouble(), 'f', 4));
            inputPlateau2->setText(QString::number(data.value(saveTemplate.value(11)).toDouble(), 'f', 4));
            inputPlateau3->setText(QString::number(data.value(saveTemplate.value(12)).toDouble(), 'f', 4));
            inputPlateau4->setText(QString::number(data.value(saveTemplate.value(13)).toDouble(), 'f', 4));
            inputCS->setText(QString::number(data.value(saveTemplate.value(14)).toDouble(), 'f', 4));
            inputCF->setText(QString::number(data.value(saveTemplate.value(15)).toDouble(), 'f', 3));
            inputFPA->setText(QString::number(data.value(saveTemplate.value(16)).toDouble(), 'f', 4));
            inputBL->setText(QString::number(data.value(saveTemplate.value(17)).toDouble(), 'f', 3));
            calculateData( );
        }
        inputControl->setEnabled(false);
        inputSerial->setEnabled(false);
        inputCF->setEnabled(false);
        inputFPA->setEnabled(false);

    }
    delete inputD;
    //QMessageBox::information(this, tr("Data Loaded"), tr("Control: C%1").arg(fileName));
}

void MountCS::saveData() {
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
		QMessageBox::warning(this, tr("Input Error"),
							tr("Control Number must be 10 digits. %1").arg(text));
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

void MountCS::clearData()
{
    dataLoaded = false;
    if( inputFPA->text().isEmpty() && inputCF->text().isEmpty()
            && inputBL->text().isEmpty() && inputPlateau1->text().isEmpty()
            && inputPlateau2->text().isEmpty() && inputPlateau3->text().isEmpty()
            && inputPlateau4->text().isEmpty() && inputCS->text().isEmpty() ) {
        QMessageBox::warning(this, tr("Clear Error!!"), tr("No data to clear."));
        inputCS->setPlaceholderText("X.XXXX");
        inputPlateau1->setEnabled(true);
        inputPlateau2->setEnabled(true);
        inputPlateau3->setEnabled(true);
        inputPlateau4->setEnabled(true);
        inputCS->setEnabled(true);
        return;
    } else if (!plateauCalc) {
        inputPlateau1->setEnabled(true);
        inputPlateau2->setEnabled(true);
        inputPlateau3->setEnabled(true);
        inputPlateau4->setEnabled(true);
    } else {
        inputCS->setEnabled(true);
    }
    inputControl->setEnabled(true);
    inputSerial->setEnabled(true);
    inputPlateau1->clear();
    inputPlateau2->clear();
    inputPlateau3->clear();
    inputPlateau4->clear();
    inputCS->clear();
    inputFPA->clear();
    inputFPA->setEnabled(true);
    inputCF->clear();
    inputCF->setEnabled(true);
    inputBL->clear();
    outputHeight->clear();
    outputHeight->setStyleSheet("");
    outputParallel->clear();
    outputParallel->setStyleSheet("");
}

void MountCS::calculateData()
{
    if ( inputPlateau1->text().isEmpty() && inputPlateau2->text().isEmpty()
                && inputPlateau3->text().isEmpty() && inputPlateau4->text().isEmpty()
                && inputCS->text().isEmpty() ) {
        QMessageBox::warning(this, tr("Calculate Error!!"), tr("No input data given."));
        return;
    } else if ( inputPlateau1->text().isEmpty() || inputPlateau2->text().isEmpty()
         || inputPlateau3->text().isEmpty() || inputPlateau4->text().isEmpty() ) {
        if ( inputCS->text().isEmpty()) {
                QMessageBox::warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
                return;
        } else if ( !inputCS->text().isEmpty() && inputCS->text().toDouble()) {
            plateauCalc = false;
            inputPlateau1->setEnabled(false);
            inputPlateau2->setEnabled(false);
            inputPlateau3->setEnabled(false);
            inputPlateau4->setEnabled(false);
        } else {
            QMessageBox::warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
            return;
        }
    } else {
        if( !inputPlateau1->text().toDouble() || !inputPlateau2->text().toDouble()
        || !inputPlateau3->text().toDouble() || !inputPlateau4->text().toDouble() ) {
            QMessageBox::warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
            return;
        }
        plateauCalc = true;
        inputCS->setEnabled(false);
        outputHeight->clear();
        outputHeight->setStyleSheet("");
        double avg;
        double parallel;
        double plat1 = inputPlateau1->text().toDouble();
        double plat2 = inputPlateau2->text().toDouble();
        double plat3 = inputPlateau3->text().toDouble();
        double plat4 = inputPlateau4->text().toDouble();

        avg = ( abs(plat1) + abs(plat2) + abs(plat3) + abs(plat4) ) / 4;

        inputCS->setText(QString::number(avg, 'f', 4));
        QList<double> plateaus;
        plateaus << plat1 << plat2 << plat3 << plat4;
        std::sort( plateaus.begin(), plateaus.end() );

        parallel = abs( plateaus.back() - plateaus.front() );

        outputParallel->setText(QString::number(parallel, 'f', 4));
        if ( parallel > 0.0020 )
            outputParallel->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputParallel->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
    if ( inputFPA->text().isEmpty() && inputCF->text().isEmpty() && inputBL->text().isEmpty() ) {
        return;
    } else if( inputCS->text().isEmpty() || inputFPA->text().isEmpty()
               || inputCF->text().isEmpty() || inputBL->text().isEmpty() ) {
        QMessageBox::warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
        return;
    } else if( !inputCS->text().toDouble() || !inputFPA->text().toDouble()
               || !inputCF->text().toDouble() || !inputBL->text().toDouble() ) {
        QMessageBox::warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
        return;
    } else {
        double sum;
        double cs = inputCS->text().toDouble();
        double fpa = inputFPA->text().toDouble();
        double cf = inputCF->text().toDouble();
        double bl = inputBL->text().toDouble();

        sum = abs(cs) - abs(fpa) + abs(cf) + abs(bl);

        outputHeight->setText(QString::number(sum, 'f', 4));
        if(sum > 5.6013 || sum < 5.5933)
            outputHeight->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputHeight->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
}

QMap<int, QString> MountCS::updateSaveTable( ) {
    saveTable[1] = inputControl->text();
    saveTable[2] = inputSerial->text();
    saveTable[10] = inputPlateau1->text();
    saveTable[11] = inputPlateau2->text();
    saveTable[12] = inputPlateau3->text();
    saveTable[13] = inputPlateau4->text();
    saveTable[14] = inputCS->text();
    saveTable[15] = inputCF->text();
    saveTable[16] = inputFPA->text();
    saveTable[17] = inputBL->text();
    saveTable[18] = outputHeight->text();
    saveTable[19] = outputParallel->text();

    return saveTable;
}

bool MountCS::fileExists( QString path ) {
    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile())
        return true;
    else
        return false;
}

MountCS::~MountCS()
{
    delete ui;
}
