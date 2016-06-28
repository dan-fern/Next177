/* mountcs.cpp contains main callouts for coldshield mounting calculator.
 *
 * loadData() does some basic error checking, loads a .csv file, populates the appropriate fields.
 * Control and dataform numbers are passed to the ProteusLookup class so that PHR history can be
 * downloaded via ProteusLookup::proteusFetch().
 *
 * saveData() checks for duplicate data, updates the saveTable, and writes the saveTable contents
 * to a .csv file.
 *
 * clearData() clears all fields and resets the dataLoaded boolean.
 *
 * calculateData() checks that all required fields are populated and then calculates Coldshield
 * Height, expected ICD, and Parallelism.  The function is structured to either take in an input
 * average coldshield height from inputCS, or to calculate an average height for inputCS from
 * inputPlateau1, inputPlateau2, inputPlateau3, and inputPlateau4.  The calculated values are then
 * checked against the design spec and color-coded accordingly.
 *
 * getScreenShot() takes a screenshot of the current window and saves to a desired directory.
 *
 * showNotepad(), showCalculations(), showBuildData(), showTutorial(), showAbout() all reference
 * functions in the ViewBuildData class.
 *
 * checkProteusData() calls the ProteusLookup class to verify that the data in the calculator
 * matches the production data saved in the PHR.  checkProteusData() is segregated by dataform.
 * It is called by way of the signal/slot in the constructor.
 *
 * initializeTables() sets up the save tables structures for load/save.
 *
 * updateSaveTable() updates the save tables before writing to .csv.
 *
 * fileExists() and checkText() are error checking functions.
*/

#include "mountcs.h"
#include "ui_mountcs.h"

MountCS::MountCS(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MountCS)
{
    ui->setupUi(this);
    // set up local variables from XML in .ui file
    inputControl = MountCS::findChild<QLineEdit *>("lineEditControl");
    inputSerial = MountCS::findChild<QLineEdit *>("lineEditSerial");
    inputCS = MountCS::findChild<QLineEdit *>("lineEditCS");
    inputFPA = MountCS::findChild<QLineEdit *>("lineEditFPA");
    inputCF = MountCS::findChild<QLineEdit *>("lineEditCF");
    inputBL = MountCS::findChild<QComboBox *>("comboBoxBL");
    inputPlateau1 = MountCS::findChild<QLineEdit *>("lineEditPlateau1");
    inputPlateau2 = MountCS::findChild<QLineEdit *>("lineEditPlateau2");
    inputPlateau3 = MountCS::findChild<QLineEdit *>("lineEditPlateau3");
    inputPlateau4 = MountCS::findChild<QLineEdit *>("lineEditPlateau4");
    outputHeight = MountCS::findChild<QLabel *>("labelOutputHeight");
    outputParallel = MountCS::findChild<QLabel *>("labelOutputParallel");
    // dataLoaded is a boolean which will tell whether data has been loaded
    dataLoaded = false;
    // this is the saving table template path, then tables are initialized
    pathTemplate = new QString("control/saveTemplate.csv");
    initializeTables( pathTemplate );
    controlInputDialog = new QInputDialog();
    kickBox = new QMessageBox();
    viewBuildData = new ViewBuildData();
    proteus = new ProteusLookup();
    // connect signal from ProteusLookup class that data has been downloaded, SLOT checks text
    connect(proteus, SIGNAL(returnText1061(QString*)), this, SLOT(checkProteusData1061()));
}

void MountCS::loadData() {
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
    // fetch data from proteus, ProteusLookup::proteusFetch( string );
    proteus->control = "C" + loadText;
    proteus->proteusFetch( "1061" );
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
        if (!data.contains("****")) {
            inputCF->setText(QString::number(saveTable[15].toDouble(), 'f', 3));
            inputFPA->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
        } else {
            inputPlateau1->setText(QString::number(data.value(saveTemplate[10]).toDouble(), 'f', 4));
            inputPlateau2->setText(QString::number(data.value(saveTemplate[11]).toDouble(), 'f', 4));
            inputPlateau3->setText(QString::number(data.value(saveTemplate[12]).toDouble(), 'f', 4));
            inputPlateau4->setText(QString::number(data.value(saveTemplate[13]).toDouble(), 'f', 4));
            inputCS->setText(QString::number(data.value(saveTemplate[14]).toDouble(), 'f', 4));
            inputCF->setText(QString::number(data.value(saveTemplate[15]).toDouble(), 'f', 3));
            inputFPA->setText(QString::number(data.value(saveTemplate[16]).toDouble(), 'f', 4));
            if (inputBL->findText(data.value(saveTemplate[17]))==-1)
                inputBL->addItem(data.value(saveTemplate[17]));
            inputBL->setCurrentIndex(inputBL->findText(data.value(saveTemplate[17])));
            // once data loaded, calculate end values and lock control number and serial number fields.
            calculateData( );
        }
        inputControl->setEnabled(false);
        inputSerial->setEnabled(false);
        inputCF->setEnabled(false);
        inputFPA->setEnabled(false);
    }
    //rawProteusText = proteus->rawText1061;
}

void MountCS::saveData() {
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

void MountCS::clearData() {
    dataLoaded = false;
    // error if no fields populated, set enabled toggled to active in case incorrectly disabled
    if( inputFPA->text().isEmpty() && inputCF->text().isEmpty()
            && inputCS->text().isEmpty() && inputPlateau1->text().isEmpty()
            && inputPlateau2->text().isEmpty() && inputPlateau3->text().isEmpty()
            && inputPlateau4->text().isEmpty() ) {
        kickBox->warning(this, tr("Clear Error!!"), tr("No data to clear."));
        inputCS->setPlaceholderText("X.XXXX");
        inputPlateau1->setEnabled(true);
        inputPlateau2->setEnabled(true);
        inputPlateau3->setEnabled(true);
        inputPlateau4->setEnabled(true);
        inputCS->setEnabled(true);
        return;
    } else if (!plateauCalc) {
        // used if calculating average coldshield height
        inputPlateau1->setEnabled(true);
        inputPlateau2->setEnabled(true);
        inputPlateau3->setEnabled(true);
        inputPlateau4->setEnabled(true);
    } else {
        // used if inputting average (not calculating) coldshield height
        inputCS->setEnabled(true);
    }
    // clear text, clear format, reenable all fields, and reset save tables
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
    inputBL->setCurrentIndex(0);
    outputHeight->clear();
    outputHeight->setStyleSheet("");
    outputParallel->clear();
    outputParallel->setStyleSheet("");
    initializeTables( pathTemplate );
}

void MountCS::calculateData() {
    // check for usable data before calculating
    if ( inputPlateau1->text().isEmpty() && inputPlateau2->text().isEmpty()
                && inputPlateau3->text().isEmpty() && inputPlateau4->text().isEmpty()
                && inputCS->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("No input data given."));
        return;
    // conditional if no plateau heights (4) are present (coldshield height input, not calculated)
    } else if ( inputPlateau1->text().isEmpty() || inputPlateau2->text().isEmpty()
         || inputPlateau3->text().isEmpty() || inputPlateau4->text().isEmpty() ) {
        if ( inputCS->text().isEmpty()) {
                kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
                return;
        } else if ( !inputCS->text().isEmpty() && inputCS->text().toDouble()) {
            // average coldshield height is input, not calculated, plateau fields are locked out
            // code will skip down to try and calculate expected ICD Height, or sum, not parallelism
            plateauCalc = false;
            inputPlateau1->setEnabled(false);
            inputPlateau2->setEnabled(false);
            inputPlateau3->setEnabled(false);
            inputPlateau4->setEnabled(false);
        } else {
            kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
            return;
        }
    } else {
        if( !inputPlateau1->text().toDouble() || !inputPlateau2->text().toDouble()
        || !inputPlateau3->text().toDouble() || !inputPlateau4->text().toDouble() ) {
            kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
            return;
        }
        // plateau heights (4) will be used to calculate average coldshield height
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

        // parallelism only possible if (4) plateau heights input to calculate average height
        inputCS->setText(QString::number(avg, 'f', 4));
        QList<double> plateaus;
        plateaus << plat1 << plat2 << plat3 << plat4;
        std::sort( plateaus.begin(), plateaus.end() );

        parallel = abs( plateaus.back() - plateaus.front() );

        outputParallel->setText(QString::number(parallel, 'f', 4));
        // once calculated, populate output objects and color-code according to spec
        if ( parallel > 0.0020 )
            outputParallel->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputParallel->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
    // now do expected ICD Height, or sum
    if ( inputFPA->text().isEmpty() && inputCF->text().isEmpty() ) {
        return;
    } else if( inputCS->text().isEmpty() || inputFPA->text().isEmpty()
               || inputCF->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
        return;
    } else if( !inputCS->text().toDouble() || !inputFPA->text().toDouble()
               || !inputCF->text().toDouble() || !inputBL->currentText().toDouble() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
        return;
    } else {
        // sum calculated here, inputCS value comes from either typed input or calculated avg
        // based on above conditionals
        double sum;
        double cs = inputCS->text().toDouble();
        double fpa = inputFPA->text().toDouble();
        double cf = inputCF->text().toDouble();
        double bl = inputBL->currentText().toDouble();

        sum = abs(cs) - abs(fpa) + abs(cf) + abs(bl);

        outputHeight->setText(QString::number(sum, 'f', 4));
        // once calculated, populate output objects and color-code according to spec
        if(sum > 5.6013 || sum < 5.5933)
            outputHeight->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputHeight->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
}

void MountCS::getScreenShot() {
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

void MountCS::showNotepad() {
    viewBuildData->showNotePad();
}

void MountCS::showCalculations() {
    viewBuildData->showLink( QString("calcs") );
}

void MountCS::showBuildData() {
    viewBuildData->showTable( saveTemplate, saveTable );
    viewBuildData->show();
}

void MountCS::showTutorial() {
    viewBuildData->showLink( QString("tutorial") );
}

void MountCS::showAbout() {
    QString exeName = "ColdshieldMount.exe\n";
    viewBuildData->showAbout( exeName );
}

void MountCS::checkProteusData1061( ) {
    // when ProteusLookup::replyFinished1061 finishes, it signals this function to check pulled text
    // inputFPA is passed to checkFectchedText() to compare it to what is in the PHR.
    proteus->checkFetchedText(inputFPA->text(), 1061);
}

void MountCS::initializeTables( QString* path ) {
    // reset all tables at .exe launch or during clearData(), etc
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
    saveTemplate.prepend("@@@@");
    saveTable.prepend("@@@@");
    templat.close();
}

void MountCS::updateSaveTable( ) {
    // get all current data from calulator fields and insert in saveTable array
    saveTable[1] = inputControl->text();
    saveTable[2] = inputSerial->text();
    saveTable[10] = inputPlateau1->text();
    saveTable[11] = inputPlateau2->text();
    saveTable[12] = inputPlateau3->text();
    saveTable[13] = inputPlateau4->text();
    saveTable[14] = inputCS->text();
    saveTable[15] = inputCF->text();
    saveTable[16] = inputFPA->text();
    saveTable[17] = inputBL->currentText();
    saveTable[18] = outputHeight->text();
    saveTable[19] = outputParallel->text();
    // asterisks are used to tell saveData() in next calculator whether file is new or not
    // default in saveTemplate is "$$$$", which is overwritten to "****", etc.
    saveTable[20] = "****";
    saveTemplate[20] = "****";
}

bool MountCS::fileExists( QString path ) {
    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile())
        return true;
    else
        return false;
}

QString MountCS::checkText( QString text ) {
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

MountCS::~MountCS()
{
    delete inputControl;
    delete inputSerial;
    delete inputCS;
    delete inputFPA;
    delete inputCF;
    delete inputBL;
    delete inputPlateau1;
    delete inputPlateau2;
    delete inputPlateau3;
    delete inputPlateau4;
    delete outputHeight;
    delete outputParallel;
    delete pathTemplate;
    delete controlInputDialog;
    delete kickBox;
    //delete rawProteusText;
    delete viewBuildData;
    delete proteus;
    delete ui;
}
