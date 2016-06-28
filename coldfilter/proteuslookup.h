#ifndef PROTEUSLOOKUP_H
#define PROTEUSLOOKUP_H

#include <QMainWindow>
#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QMessageBox>
#include <iostream>

class QTextEdit;

namespace Ui {
class ProteusLookup;
}

class ProteusLookup : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProteusLookup(QWidget *parent = 0);
    QString control;
    QString dataform;
    void testFetch( );
    void proteusFetch( QString );
    QString *rawText1061;
    QString *rawText1065;
    ~ProteusLookup();

public slots:
    void replyFinished1061(QNetworkReply*);
    void replyFinished1065(QNetworkReply*);
    void checkFetchedText( QString, int );

signals:
    // signals are sent to other classes to signify text has been downloaded from the PHR
    void returnText1061(QString*);
    void returnText1065(QString*);

private:
    Ui::ProteusLookup *ui;
    int currentDataform;
    QNetworkAccessManager *m_manager_1061;
    QNetworkAccessManager *m_manager_1065;
};

#endif // PROTEUSLOOKUP_H
