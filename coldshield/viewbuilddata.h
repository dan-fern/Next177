#ifndef VIEWBUILDDATA_H
#define VIEWBUILDDATA_H

#include <QWidget>
#include <QtGui>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>

class QLabel;
class QLineEdit;
class QTextEdit;
class QTableView;

namespace Ui {
class ViewBuildData;
}

class ViewBuildData : public QWidget
{
    Q_OBJECT

public:
    explicit ViewBuildData(QWidget *parent = 0);
    void showNotePad( );
    void showLink( QString );
    void showTable( QList <QString>, QList <QString> );
    void showAbout( QString );
    ~ViewBuildData();

private:
    Ui::ViewBuildData *ui;
    QLineEdit *inputControl;
    QLineEdit *inputSerial;
    QMessageBox *kickBox;
    QTextEdit *notePad;
    QTableWidget *tableView;
};

#endif // VIEWBUILDDATA_H
