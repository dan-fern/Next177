/* ProteusLookup class is shared code used in multiple calculators to handle communications with
 * the Proteus server.  It is designed to take a control number and dataform and pull relevant text.
 * Overall, the function is pretty clunky, and it calls functions with dataforms hardcoded in. This
 * happened because I struggled with asynchronicity of the network requests with the main code.
 * In a nutshell, the network replies were out of sync with their return calls in the main code.  To
 * circumvent this, I added several signals/slots to make sure functions were called sequentially.
 *
 * testFetch() is defunct.  It was used in preliminary URL and call tests.  It was left in as an
 * ideal sandbox function for future maintenance.
 *
 * proteusFetch() is called in the main class.  It takes in a dataform, combines with the set control
 * number, and constructs the call URL.  That URL is passed to the appropriate network manager based
 * on dataform.
 *
 * replyFinished1061() and replyFinished1065() are called sequentially when the URL call from
 * proteusFetch() completes.  This is called using the signal/slot connection in the constructor.
 * When called, replyFinished() converts the returned data to a string, and then signals the main
 * class that it is ready to compare data.  See MountCS::checkProteusData1061(),
 * MountCS::checkProteusData1065(), MountCF::checkProteusData1061(), MountCF::checkProteusData1065()
 *
 * checkFetchedText() takes in text and a dataform from the main class.  It compares the input text
 * to the downloaded text from the PHR.  Most of this function is locating indices and formatting.
*/

#include "proteuslookup.h"
#include "ui_proteuslookup.h"

ProteusLookup::ProteusLookup(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProteusLookup)
{
    ui->setupUi(this);
    // to store raw text from both dataforms
    rawText1061 = new QString;
    rawText1065 = new QString;
    // manages network communications for both dataforms
    m_manager_1061 = new QNetworkAccessManager(this);
    m_manager_1065 = new QNetworkAccessManager(this);

    // when the reply pointer has done downloading, it signals replyFinished to convert to string
    connect(m_manager_1061, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(replyFinished1061(QNetworkReply*)));
    connect(m_manager_1065, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(replyFinished1065(QNetworkReply*)));
}

void ProteusLookup::testFetch( ) {
    //QUrl url("http://google.com");
    //QUrl url("http://sbfdb/proteus/application/admin.php?page=GenericService&sender=COCReport&controlNbr=C2001933086");
    //QNetworkRequest request;
    //QSslConfiguration config(QSslConfiguration::defaultConfiguration());
    //request.setSslConfiguration(config);
    //request.setUrl(url);
    //m_manager->get(request);
    //m_manager->get(QNetworkRequest(url));
    return;
}

void ProteusLookup::proteusFetch( QString newDataform ) {
    // update ProteusLookup dataform
    dataform = newDataform;
    currentDataform = dataform.toInt();
    QString urlStr1 = "http://sbfdb/proteus/application/admin.php?page=GenericService&sender=dataFormResult&controlNbr=";
    QString urlStr2 = "&dataForm=";
    // construct url
    QUrl url(urlStr1 + control + urlStr2 + dataform);
    // pass url to appropriate network manager
    switch (currentDataform) {
    case 1061:  m_manager_1061->get(QNetworkRequest(url)); break;
    case 1065:  m_manager_1065->get(QNetworkRequest(url)); break;
    default: QMessageBox::information(this, tr("Dataform Error"), "Could not find dataform.");
    }
}

void ProteusLookup::replyFinished1061( QNetworkReply *pReply ) {
    // when data downloaded, replyFinished is signalled and converts data to string
    QByteArray data = pReply->readAll();
    *rawText1061 = QString(data);
    // signal main class that it is ready to compare downloaded PHR text to calculator text
    emit returnText1061(rawText1061);
}

void ProteusLookup::replyFinished1065( QNetworkReply *pReply ) {
    // when data downloaded, replyFinished is signalled and converts data to string
    QByteArray data = pReply->readAll();
    *rawText1065 = QString(data);
    // signal main class that it is ready to compare downloaded PHR text to calculator text
    emit returnText1061(rawText1065);
}

void ProteusLookup::checkFetchedText( QString inputText, int dataform ) {
    // receive inputText (calculator text) and dataform, compare to downloaded PHR text
    QString checkProteusText;
    switch (dataform) {
    case 1061: {
        // get current proteus text
        QString rawText = *rawText1061;
        // get index to start of useful data
        QString searchText("MPPStepMountFPAMB_DATAFORM1061_Datum__dash_A_dash__to_Optical_Center_Height = ");
        int searchInd = rawText.indexOf(searchText, Qt::CaseInsensitive) + searchText.length();
        // parse text for value
        while (rawText.at(searchInd) != 'M')
            checkProteusText = checkProteusText + rawText.at(searchInd++);
        checkProteusText = checkProteusText.trimmed();
        // check proteus data with input data, return if no issue
        if (checkProteusText == inputText || checkProteusText.toDouble() == inputText.toDouble())
            return;
        else
            QMessageBox::warning(this, tr("Data Load Error"), tr("Centerline data loaded from Calculator: %1 "
                                                                "\nCenterline data loaded from Proteus PHR: %2 "
                                                                "\nData does not appear to match."
                        "\nVerify data in calculator with PHR before proceeding with assembly.").arg(inputText).arg(checkProteusText));
        break;
    }
    case 1065: {
        // get current proteus text
        QString rawText = *rawText1065;
        // get index to start of useful data
        QString searchText("MPPStepMountColdshield_DATAFORM1065_Datum__dash_A_dash__to_Coldshield_Pedestal__leftParen_CURE_rightParen_ = ");
        int searchInd = rawText.indexOf(searchText, Qt::CaseInsensitive) + searchText.length();
        // parse text for value
        while (rawText.at(searchInd) != 'M')
            checkProteusText = checkProteusText + rawText.at(searchInd++);
        checkProteusText = checkProteusText.trimmed();
        // check proteus data with input data, return if no issue
        if (checkProteusText == inputText || checkProteusText.toDouble() == inputText.toDouble())
            return;
        else
            QMessageBox::warning(this, tr("Data Load Error"), tr("Coldshield Height data loaded from Calculator: %1 "
                                                                "\nColdshield Height loaded from Proteus PHR: %2 "
                                                                "\nData does not appear to match."
                        "\nVerify data in calculator with PHR before proceeding with assembly.").arg(inputText).arg(checkProteusText));
        break;
    }
    default:
        QMessageBox::information(this, tr("Dataform Error"), "Could not find dataform.");
    }
}

ProteusLookup::~ProteusLookup()
{
    delete rawText1061;
    delete rawText1065;
    delete ui;
}
