/* mountcf.cpp contains main callouts for coldfilter mounting calculator.
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
 * calculateData1() takes in the measured coldfilter thickness, adds it to the loaded coldshield
 * height, subtracts the loaded optical centerline and suggests coldfilter epoxy bondlines from a
 * range of [0.001, 0.0015, 0.002, 0.0025, 0.003] to get the build in spec.  The algebra used is
 * designed to favor a 0.001" bondline when possible.  Once the bondline is output, corresponding
 * tool ball height and expected ICD are also output.  The calculated values are then checked
 * against the design spec and color-coded accordingly.
 *
 * calculateData2() checks that all required fields are populated and then calculates Coldfilter
 * Height, final ICD, and CF Parallelism.  The function is structured to either take in an input
 * average coldfilter height from inputCF, or to calculate an average height for inputCF from
 * inputFiducial1, inputFiducial2, and inputFiducial3.  The calculated values are then checked
 * against the design spec and color-coded accordingly.
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
 * updateSaveTable() updates the save tables before writing to .csv.  calc1 and calc2 booleans are
 * passed to allow mid-assembly saves.
 *
 * fileExists() and checkText() are error checking functions.
*/

#include "mountcf.h"
#include "ui_mountcf.h"

MountCF::MountCF(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MountCF)
{
    ui->setupUi(this);
    // set up local variables from XML in .ui file
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
    // calc1, calc2 tell the saveTables which data should be updated.
    calc1 = false;
    calc2 = false;
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
    connect(proteus, SIGNAL(returnText1065(QString*)), this, SLOT(checkProteusData1065()));
}

void MountCF::loadData() {
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
    proteus->proteusFetch( "1065" );
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
        // now separate which half or halves of the calculator should be populated
        if (!data.contains("*****")) {
            //no previous CF calc data, only data from previous calculator used
            inputCS->setText(QString::number(data.value(saveTemplate[14]).toDouble(), 'f', 4));
            inputFPA1->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
            inputFPA2->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
        } else {
            //only calc1 CF data, left half of UI
            inputCF1->setText(QString::number(data.value(saveTemplate[21]).toDouble(), 'f', 4));
            inputCS->setText(QString::number(data.value(saveTemplate[22]).toDouble(), 'f', 4));
            inputFPA1->setText(QString::number(data.value(saveTemplate[23]).toDouble(), 'f', 4));
            //once populated, calculate end values and lock control number and serial number fields.
            calculateData1( );
            inputFPA2->setText(QString::number(data.value(saveTemplate[8]).toDouble(), 'f', 4));
        }
        inputCS->setEnabled(false);
        inputFPA1->setEnabled(false);
        if (data.contains("******")) {
            //calc1 and calc2 data both exist, both halves of calculator filled
            inputFiducial1->setText(QString::number(data.value(saveTemplate[28]).toDouble(), 'f', 4));
            inputFiducial2->setText(QString::number(data.value(saveTemplate[29]).toDouble(), 'f', 4));
            inputFiducial3->setText(QString::number(data.value(saveTemplate[30]).toDouble(), 'f', 4));
            inputCF2->setText(QString::number(data.value(saveTemplate[31]).toDouble(), 'f', 4));
            inputFPA2->setText(QString::number(data.value(saveTemplate[32]).toDouble(), 'f', 4));
            //once populated, calculate end values and lock control number and serial number fields.
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
    // also flag user for partial load
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
    // update all tables from current calculator fields for writing to .csv
    updateSaveTable( calc1, calc2 );
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

void MountCF::clearData() {
    dataLoaded = false;
    // error if no fields populated, set enabled toggled to active in case incorrectly disabled
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
        initializeTables( pathTemplate );
    } else if (calc2) {
        // allow to clear 2nd half of calculator while leaving 1st half
        calc2 = false;
        inputFiducial1->clear();
        inputFiducial2->clear();
        inputFiducial3->clear();
        inputCF2->clear();
        inputFPA2->clear();
        inputFPA2->setEnabled(true);
        if (!fiducialCalc) {
            // used if calculating average of fiducial heights
            inputFiducial1->setEnabled(true);
            inputFiducial2->setEnabled(true);
            inputFiducial3->setEnabled(true);
        } else {
            // used if inputting average (not calculating) fiducial heights
            inputCF2->setEnabled(true);
        }
        // clear text, clear format for 2nd half
        outputHeight2->clear();
        outputHeight2->setStyleSheet("");
        outputParallel->clear();
        outputParallel->setStyleSheet("");
    } else if (calc1) {
        // clears 1st half if 2nd half empty or previosuly cleared
        calc1 = false;
        // clear text, clear format, reenable all fields, and reset save tables
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
        initializeTables( pathTemplate );
    } else {
        // if no calculated, still clear all fields
        inputFiducial1->clear();
        inputFiducial2->clear();
        inputFiducial3->clear();
        inputCF2->clear();
        inputFPA2->clear();
        inputCF1->clear();
        inputCS->clear();
        inputFPA1->clear();
    }
}

void MountCF::calculateData1() {
    // check for usable data before calculating, 1st half
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
        // create array of possible coldfilter bondlines
        double bondline [] = { 0.0010, 0.0015, 0.0020, 0.0025, 0.0030 };
        // create vectors for bondlines, ball heights, and associated sums
        std::vector<double> bond;
        bond.assign(bondline, bondline+5);
        std::vector<double> balls (bond.size());
        std::vector<double> sum (bond.size());
        double cf = inputCF1->text().toDouble();
        double cs = inputCS->text().toDouble();
        double fpa = inputFPA1->text().toDouble();
        int choice;

        // loop over size of possible coldfilter bondlines (5) and find bond value which gets build
        // closest to spec.  Written to favor a 0.001" bondline when possible
        for ( int i = 0; i < 5; i++ ) {
            sum[i] = abs(cf) + abs(cs) - abs(fpa) + abs(bond[i]);
            balls[i] =  sum[i] + abs(fpa);
            if (i == 0)
                choice = 0;
            else if ( abs(5.5941 - sum[i]) < abs(5.5941 - sum[choice]) )
                choice = i;
        }

        // once choice is determined, bondline at index choice is selected for build.  Ball height
        // and expected ICD associated with that bondline are also given.
        outputBond->setText(QString::number(bond[choice], 'f', 4));
        outputBalls->setText(QString::number(balls[choice], 'f', 4));
        outputHeight1->setText(QString::number(sum[choice], 'f', 4));
        if( sum[choice] > 5.6013 || sum[choice] < 5.5933 ) {
            // if no good bondline, kick out
            outputBalls->clear();
            outputBond->clear();
            outputHeight1->setText(QString::number(sum[choice], 'f', 4));
            outputHeight1->setStyleSheet("QLabel { background-color : red; color : black; }");
            kickBox->critical(this, tr("ICD not met"),
                        tr("No possible bond line.\nExpected Height: %1").arg(sum[choice]));
        } else if( sum[choice] == 5.6013 || sum[choice] == 5.5933 ) {
            // ICD barely met.  Flags user to take extreme caution
            outputHeight1->setStyleSheet("QLabel { background-color : yellow; color : black; }");
            kickBox->warning(this, tr("ICD met at critical dimension"),
                        tr("Expected ICD height is at extreme of allowable range.\n"
                           "Expected Height: %1").arg(sum[choice]));
        } else {
            // otherwise, all is well, proceed with build
            outputHeight1->setStyleSheet("QLabel { background-color : green; color : black; }");
            return;
        }
        /*
        } else if ( choice > 1 && abs(5.5973 - sum[choice]) <= bondline[0]/2 ) {
            choice = choice - 1;
        } else if ( choice > 1 && abs(5.5973 - sum[choice]) <= bondline[0] ) {
            choice = choice - 2;

        std::cout << bond[0] << " " << bond[1] << " " << bond[2] << " " << bond[4] << std::endl;
        std::cout << sum[0] << " " << sum[1] << " " << sum[2] << " " << sum[4] << std::endl;
        std::cout << balls[0] << " " << balls[1] << " " << balls[2] << " " << balls[4] << std::endl;
        std::cout << bond[choice] << " " << sum[choice] << " " << balls[choice] << std::endl;
        */
        //QMessageBox::warning(this, tr("BILLY'S HERE!!"), tr("Size: %1").arg(balls.size()));
        return;
    }
}

void MountCF::calculateData2() {
    // check for usable data before calculating 2nd half
    if ( inputFiducial1->text().isEmpty() && inputFiducial2->text().isEmpty()
                && inputFiducial3->text().isEmpty() && inputCF2->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("No input data given."));
        return;
    // conditional if no fiducial heights (3) are present (coldfilter height input, not calculated)
    } else if ( inputFiducial1->text().isEmpty() || inputFiducial2->text().isEmpty()
                || inputFiducial3->text().isEmpty() ) {
        if ( inputCF2->text().isEmpty() || inputFPA2->text().isEmpty() ) {
            kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
            return;
        } else if ( !inputCF2->text().isEmpty() && inputCF2->text().toDouble()) {
            calc2 = true;
            // average coldfilter height is input, not calculated, plateau fields are locked out
            // code will skip down to try and calculate final ICD Height, or sum, and not parallelism
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
        // fiducial heights (3) will be used to calculate average coldfilter height
        fiducialCalc = true;
        inputCF2->setEnabled(false);
        outputHeight2->clear();
        outputHeight2->setStyleSheet("");
        double avg;
        double parallel;
        double fid1 = inputFiducial1->text().toDouble();
        double fid2 = inputFiducial2->text().toDouble();
        double fid3 = inputFiducial3->text().toDouble();
        // parallelism only possible if (3) fiducial heights input to calculate average height
        QList<double> fiducials;
        fiducials << fid1 << fid2 << fid3;
        std::sort( fiducials.begin(), fiducials.end() );

        avg = ( abs(fid1) + abs(fid2) + abs(fid3) ) / fiducials.size();

        parallel = abs( fiducials.back() - fiducials.front() );

        inputCF2->setText(QString::number(avg, 'f', 4));
        outputParallel->setText(QString::number(parallel, 'f', 4));
        // once calculated, populate output objects and color-code according to spec
        if ( parallel > 0.0030 )
            outputParallel->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputParallel->setStyleSheet("QLabel { background-color : green; color : black; }");
        }
    // now do final ICD Height, or sum
    if ( inputFPA2->text().isEmpty() ) {
        return;
    } else if( inputCF2->text().isEmpty() || inputFPA2->text().isEmpty() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Not enough data to calculate."));
        return;
    } else if( !inputCF2->text().toDouble() || !inputFPA2->text().toDouble() ) {
        kickBox->warning(this, tr("Calculate Error!!"), tr("Data must be numeric."));
        return;
    } else {
        // sum calculated here, inputCF value comes from either typed input or calculated avg
        // based on above conditionals
        double sum;
        double cf = inputCF2->text().toDouble();
        double fpa = inputFPA2->text().toDouble();

        sum = abs(cf) - abs(fpa);

        outputHeight2->setText(QString::number(sum, 'f', 4));
        // once calculated, populate output objects and color-code according to spec
        if(sum > 5.6013 || sum < 5.5933)
            outputHeight2->setStyleSheet("QLabel { background-color : red; color : black; }");
        else
            outputHeight2->setStyleSheet("QLabel { background-color : green; color : black; }");
    }
}

void MountCF::getScreenShot() {
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

void MountCF::showNotepad() {
    viewBuildData->showNotePad();
}

void MountCF::showCalculations() {
    viewBuildData->showLink( QString("calcs") );
}

void MountCF::showBuildData() {
    viewBuildData->showTable(saveTemplate, saveTable);
    viewBuildData->show();
}

void MountCF::showTutorial() {
    viewBuildData->showLink( QString("tutorial") );
}

void MountCF::showAbout() {
    QString name = "ColdfilterMount.exe\n";
    viewBuildData->showAbout( name );
}

void MountCF::checkProteusData1061( ) {
    // when ProteusLookup::replyFinished1061 finishes, it signals this function to check pulled text
    // inputFPA is passed to checkFectchedText() to compare it to what is in the PHR.
    proteus->checkFetchedText(inputFPA1->text(), 1061);
    proteus->checkFetchedText(inputFPA2->text(), 1061);
}

void MountCF::checkProteusData1065( ) {
    // when ProteusLookup::replyFinished1065 finishes, it signals this function to check pulled text
    // inputCS is passed to checkFectchedText() to compare it to what is in the PHR.
    proteus->checkFetchedText(inputCS->text(), 1065);
}

void MountCF::initializeTables( QString* path ) {
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
    saveTemplate.prepend("@@@@@");
    saveTable.prepend("@@@@@");
    templat.close();
}

void MountCF::updateSaveTable( bool calculated1, bool calculated2 ) {
    saveTable[1] = inputControl->text();
    saveTable[2] = inputSerial->text();
    // get all current data from calulator fields and insert in saveTable array
    if (calculated1) {
        saveTable[21] = inputCF1->text();
        saveTable[22] = inputCS->text();
        saveTable[23] = inputFPA1->text();
        saveTable[24] = outputBond->text();
        saveTable[25] = outputBalls->text();
        saveTable[26] = outputHeight1->text();
        // asterisks are used to tell saveData() whether file is fully or only half saved.
        // default in saveTemplate is "$$$$$", which is overwritten to "*****", etc.
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
        // asterisks are used to tell saveData() whether file is fully or only half saved.
        // default in saveTemplate is "$$$$$$", which is overwritten to "******", etc.
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

MountCF::~MountCF()
{
    delete inputControl;
    delete inputSerial;
    delete inputCF1;
    delete inputCF2;
    delete inputCS;
    delete inputFPA1;
    delete inputFPA2;
    delete inputFiducial1;
    delete inputFiducial2;
    delete inputFiducial3;
    delete outputBond;
    delete outputBalls;
    delete outputHeight1;
    delete outputHeight2;
    delete outputParallel;
    delete pathTemplate;
    delete controlInputDialog;
    delete kickBox;
    delete viewBuildData;
    delete proteus;
    delete ui;
}
