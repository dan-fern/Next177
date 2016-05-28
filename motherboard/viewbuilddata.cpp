// ViewBuildData class is shared code used in multiple calculators.  showNotepad() launches a window
// for taking notes during the build.  showCalculations() shows a .html of the calculations performed
// at that step.  showTable() outputs a window of all assembly data at that point.  showAbout()
// provides software development information.

#include "viewbuilddata.h"
#include "ui_viewbuilddata.h"

ViewBuildData::ViewBuildData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewBuildData)
{
    ui->setupUi(this);
    inputControl = ViewBuildData::findChild<QLineEdit *>("lineEditControl");
    inputSerial = ViewBuildData::findChild<QLineEdit *>("lineEditSerial");
    tableView = ViewBuildData::findChild<QTableWidget *>("tableView");
    kickBox = new QMessageBox();
    notePad = new QTextEdit();
}

void ViewBuildData::showNotePad( ) {
    notePad->show();
}

void ViewBuildData::showCalculations( QString process ) {
    // takes a string list of commands and passes to cmd to launch a browser with specific .html
    //QString path("file://sbf40010/SHARED/DEWAR/PUBLIC/DF/177/calcSupport/" + process + ".html");
    QString path("file://sbf40010/SHARED/DEWAR/PUBLIC/ENB-Dewar/ENB-DF/coldstackCalculator/calculator/support/" + process + ".html");
    QStringList commands;
    commands << "/C" << "start" << path;
    QProcess::startDetached("cmd", commands );
}

void ViewBuildData::showTable( QList<QString> tableKeys, QList<QString> tableVals ) {
    // populates a table of production data across all steps.  most of this function is formatting.
    tableView->clear();
    if(tableVals.isEmpty() || tableKeys.isEmpty()) {
        kickBox->information(this, tr("Error!!"), tr("No data to show."));
        return;
    }
    int rowCount = 0;
    inputControl->setText(tableVals[1]);
    inputSerial->setText(tableVals[2]);
    inputControl->setEnabled(false);
    inputSerial->setEnabled(false);
    bool isBold = false;
    while (rowCount != tableVals.length()-3) {
        tableView->setRowCount(rowCount+1);
        QString str1 = tableKeys[rowCount+3];
        QString str2 = tableVals[rowCount+3];
        if (str1.contains("&")) {
            str1.remove(str1.at(str1.length()-1));
            isBold = true;
        }
        QTableWidgetItem *tableKey = new QTableWidgetItem(str1);
        QTableWidgetItem *tableVal = new QTableWidgetItem(str2);
        tableView->setItem(rowCount, 0, tableKey);
        tableView->setItem(rowCount, 1, tableVal);
        if (isBold) {
            QFont font;
            font.setBold(true);
            tableView->item(rowCount, 0)->setFont(font);
            tableView->item(rowCount, 1)->setFont(font);
            isBold = false;
        }
        rowCount++;
    }
    tableView->setColumnWidth(0,175);
    tableView->setColumnWidth(1,75);
    tableView->resizeRowsToContents();
    int rowHeight = (tableView->height() - 2) / rowCount;
    int tableViewHeight = rowHeight * rowCount + 2;
    for (int i = 0; i <= rowCount; i++)
        tableView->setRowHeight(i, rowHeight);
    tableView->setFixedHeight(tableViewHeight);
}

void ViewBuildData::showAbout( QString exeName ) {
    // plug for the author :)
    QString str1 = "Copyright, Lockheed Martin MFC, 2016\n";
    QString str2 = "All rights reserved\n";
    QString str3 = "Originally authored by Daniel Fernandez\n";
    QString str4 = "http://github.com/loki8999\n";
    QString allStrings = exeName + str1 + str2 + str3 + str4;
    QMessageBox aboutDialog(QMessageBox::NoIcon, "About", allStrings);
    aboutDialog.exec();
}

ViewBuildData::~ViewBuildData()
{
    delete inputControl;
    delete inputSerial;
    delete kickBox;
    delete notePad;
    delete tableView;
    delete ui;
}
