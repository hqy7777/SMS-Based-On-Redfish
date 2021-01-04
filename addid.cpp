#include "addid.h"
#include "ui_addid.h"
#include "widget.h"

AddId::AddId(QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::AddId)
{
    //init addid ui
    initAddidUi();
}

AddId::~AddId()
{
    delete ui;
}

void AddId::initAddidUi()
{
    //set icon,name and size for window
    ui->setupUi(this);
    setWindowTitle(tr("添加服务器"));
    setWindowIcon(QIcon(":/main/servers"));
    setMouseTracking(true);
    setMaximumSize(QSize(319,417));

    //set modal window
    setWindowFlags(Qt::Window);
    setWindowModality(Qt::WindowModal);

    //set remember function of lineEdit

    //set default text
    ui->lineEdit_1->setText("");
    ui->lineEdit_2->setText("");
    ui->lineEdit_3->setText("");

    ui->lineEdit_3->setEchoMode(QLineEdit::Password);
}

//Qt rules :The painter must in paintEvent
void AddId::paintEvent(QPaintEvent *){

    //set window background
    QPainter p;
    p.begin(this);
    p.drawPixmap(rect(),QPixmap(":/skin/21"));
}

/*
 * from save button
thinking：
First，input id,username and password,
when we clicks save button,
we 'll gain the url(sessions) through the id，
through url(sessions),username and password,then post for the json data which contains x-token
through url(system) and x-token,finally get the json data which contains critical features
*/

void AddId::on_save_clicked()
{
    QRegExp rx2;
    rx2.setPattern("^[10]/./d/.[0-99]/.[0-255]$");
    QStringList ids = readId();
    if(ids[0]!="0" && ids.contains(ui->lineEdit_1->text())){
        qDebug() <<"1";
        QMessageBox::about(NULL, "提示", QString("已存在ID为%1的服务器").arg(ui->lineEdit_1->text()));
    }
    else if(ui->lineEdit_1->text().trimmed() == "" ||ui->lineEdit_2->text().trimmed() == "" || ui->lineEdit_3->text().trimmed() == "")
    {
        QMessageBox::information(this, "登录失败","服务器ID或用户名密码为空");
        ui->lineEdit_1->setFocus();
        return;
    }
//    else if(!rx2.exactMatch(ui->lineEdit_1->text()))
//    {
//        QMessageBox::information(this, "登录失败","服务器ID格式错误");
//        // clear lineedit
//        ui->lineEdit_1->clear();
//        //set cursor position
//        ui->lineEdit_1->setFocus();
//        return;
//    }
    //Verification passed
    else{
        //slot function for button save
        this->hide();

        //QNetworkAccessManager is asynchronous, and the execution of post and get is not synchronous, so it is convenient to get after executing post
        manager=new QNetworkAccessManager(this);

        //before below function ,the number of times this code writed,the number of times receiveReply run
        connect(manager, &QNetworkAccessManager::finished, this, &AddId::receiveReply);

        //loading..
        emit showLoading();

        //post for x-token when get the id from the table of save
        QString token = postToken(getTokenUrl(ui->lineEdit_1->text()));

        //"0" is a sign for check server by id_username_password
        if (token != "0"){
            //get Critical data
            QStringList criticalstr = getCriticalMsg(getSystemUrl(ui->lineEdit_1->text()),token);

            QString serveritynum = getSeverityMsg(getLogServiceUrl(ui->lineEdit_1->text()),token);

            //send signal to widget for showing the new line of the treewidget
            emit showLabel(ui->lineEdit_1->text(),criticalstr[0],criticalstr[1],serveritynum,criticalstr[2],criticalstr[3]);
        }
    }
}

QStringList AddId::readId(){

    connectDatabase();

    //read data
    QSqlQuery sql_query;

    sql_query.exec("select * from realdata");

    QStringList strs;

    if(!sql_query.exec("select count(*) from realdata"))//judge table empty or not
    {
        qDebug()<<sql_query.lastError();
        strs <<QString("0");
        return strs;
    }
    else//not empty
    {
        while(sql_query.next())
        {
            QString id = sql_query.value(0).toString();
            strs<<QString(id);
        }
        return strs;
    }
}


void AddId::connectDatabase(){

    //create and connect the database
    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        database = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("MyDataBase.db");
    }
    //open the database
    if (!database.open())
    {

        qDebug() << "11111" << "Error: Failed to connect database." << database.lastError();
    }
    else
    {
        qDebug() << "数据库连接成功！"<<"获取数据库的Id数据.";
    }
}

/*post request for sessions
id come from two ways
1,save button ui->treeWidget->lineEdit_1->text()
2,emit refresh_realtimedata(ui->treeWidget->currentItem()->text(0));
*/
QString AddId::postToken(QString id)
{
    qDebug() << id;
    // new request object
    QNetworkRequest request;

    // ready for sending https request
    QSslConfiguration config;
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(conf);
    request.setUrl(QUrl("https://www.baidu.com"));
    request.setUrl(QUrl(id));


    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    //set raw header of request
    request.setRawHeader("Content-Type", "application/json");

    //check supported agreement
    qDebug()<< manager->supportedSchemes();//("ftp", "file", "qrc", "http", "https", "data")

    //obtain form data——username and password
    QString submitMsg = QString(R"(
                                {
                                  "UserName": "%1",
                                  "Password": "%2"
                                })").arg(ui->lineEdit_2->text()).arg(ui->lineEdit_3->text());

    qDebug() <<"1";
    QNetworkReply *reply=manager->post(request,submitMsg.toUtf8());
    //open a local event loop, then wait reply but do not stop thread(significant)

    QEventLoop eventLoop;
    connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    /* for  \"X-Auth-Token\":
    analysis json data
    */
    qDebug() << "2";
    QString token = analysisSessionsJson(reply);

    qDebug() << "3";
    if(token!="0"){
        return token;
    }
    else{
        this->hide();
        emit closeLoading();
        this->show();
        return "0";
    }
}

/*get the post url for x-token
 * url1
 * https://id/redfish/v1/******
*/
QString AddId::getTokenUrl(QString url){
    return QString("https://%1/redfish/v1/*****").arg(url);
}

/*analysis SessionsJson for token
 * url1
 * https://id/redfish/v1/******
*/
QString AddId::analysisSessionsJson(QNetworkReply *reply){
    //judge if format of the json is right
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll(); //read all bytes
        QJsonParseError jsonError_login;

        //switch to json document
        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError_login);

        //analysis Json  error
        if (document.isObject())
        {
            QJsonObject obj = document.object();
            if (obj.contains("Oem"))
            {
                QJsonObject object_value = obj.value("Oem").toObject();
                if (object_value.contains("BMC")){
                    QJsonObject object1_value = object_value.value("BMC").toObject();
                    if(object1_value.contains("X-Auth-Token"))
                    {
                        QString token_val = object1_value.value("X-Auth-Token").toString();
                        return token_val;
                    }
                }
            }
        }
    }
    else
    {
        QMessageBox::information(this, "错误","解析失败");
        qDebug()<<"1"<<"handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //statusCodeV id corresponding code for http server，reply->error()is a defined wrong-code of Qt which can be finded in help of Qt
        qDebug( "found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        qDebug()<<"1111"<<reply->errorString();
        return "0";
    }
    reply->deleteLater();
}

/*get the url of server information
 * url2
 * https://id/redfish/v1/******
*/
QString AddId::getSystemUrl(QString url)
{
    url = QString("https://%1/redfish/v1/Systems/******").arg(url);
    return url;
}


/*get request for system infomation
 * url2
 * https://id/redfish/v1/******
*/
QStringList AddId::getCriticalMsg(QString url,QString token)
{
    QNetworkRequest request;

    QSslConfiguration config;
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(conf);
    request.setUrl(QUrl("https://www.baidu.com"));

    //set url and xAuthToken
    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setRawHeader(QByteArray("X-Auth-Token"),QByteArray(token.toUtf8()));

    QNetworkReply *systeminfo = manager->get(request);

    //if no below three codes，will first run the latter code after QNetworkReply *systeminfo = manager->get(request);then to run reply
    QEventLoop eventLoop;
    connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QStringList criticalstr = analysisSystemJson(systeminfo);

    return criticalstr;
}

/*analysis processing
 * url2
 * https://id/redfish/v1/******
for product name,state of on or off,warnings num,GUID*/
QStringList AddId::analysisSystemJson(QNetworkReply *sys){
    QStringList criticalstr;
    if (sys->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = sys->readAll();
        QJsonParseError jsonError_system;

        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError_system);

        if (document.isObject())
        {
            QJsonObject obj = document.object();
            if (obj.contains("Manufacturer")) {
                QString manufacturer = obj.value("Manufacturer").toString();
                criticalstr.append(manufacturer);
                qDebug() << manufacturer;
            }
            if (obj.contains("PowerState")) {
                QString powerstate = obj.value("PowerState").toString();
                criticalstr.append(powerstate);
                qDebug() << powerstate;
            }
            if (obj.contains("UUID")) {
                QString uuid = obj.value("UUID").toString();
                criticalstr.append(uuid);
                qDebug() << uuid;
            }
            if (obj.contains("Status"))
            {
                QJsonObject object_value = obj.value("Status").toObject();
                if (object_value.contains("Health")){
                    QString health = object_value.value("Health").toString();
                    criticalstr.append(health);
                    qDebug() << health;
                }
            }
        }
    }
    else
    {
        QMessageBox::information(this, "Warning","Analysis Failed！");
        qDebug()<<"2"<<"handle errors here";
        this->show();
        //QVariant statusCodeV = sys->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //qDebug( "found error ....code: %d %d\n", statusCodeV.toInt(), (int)sys->error());
        //qDebug() <<(sys->errorString());

    }
    sys->deleteLater();
    //close loading ui
    return criticalstr;
}

/*get url of LogServices
 * url3
https://id/redfish/v1/Managers/*********
*/
QString AddId::getLogServiceUrl(QString url)
{
    url = QString("https://%1/redfish/v1/Managers/**********").arg(url);
    return url;
}

/*get request for LogServer infomation
 * url3
https://id/redfish/v1/Managers/*********
*/
QString AddId::getSeverityMsg(QString url,QString token)
{
    QNetworkRequest request;
    QSslConfiguration config;
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(conf);
    request.setUrl(QUrl("https://www.baidu.com"));
    //set url and xAuthToken
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setRawHeader(QByteArray("X-Auth-Token"),QByteArray(token.toUtf8()));

    QNetworkReply *logserverinfo = manager->get(request);

    //if no below three codes，will first run the latter code after QNetworkReply *systeminfo = manager->get(request);then to run reply
    QEventLoop eventLoop;
    connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QString healthrate = analysisLogServerJson(logserverinfo);

    return healthrate;
}

/*get request
 * url3
https://id/redfish/v1/Managers/*********
for severitynum
*/
QString AddId::analysisLogServerJson(QNetworkReply *logserver){
    int serveritynum = 0;
    int healthnum = 0;
    qint32 logcount = 0;
    if (logserver->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = logserver->readAll();
        QJsonParseError jsonError_logserver;

        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError_logserver);

        if (document.isObject())
        {
            QJsonObject obj = document.object();
            if (obj.contains("Members@odata.count"))
            {
                //note that value for "Members@odata.count" is not a string
                logcount = obj.value("Members@odata.count").toInt();
            }
            if (obj.contains("Members"))
            {
                QJsonValue members = obj.value("Members");//the value of members is a array
                if(members.isArray())
                {
                    for(int i = 0;i< logcount;i++)
                    {
                        QJsonObject members_obj = members.toArray().at(i).toObject();
                        if(members_obj.contains("Severity"))
                        {
                            QString status_value = members_obj.value("Severity").toString();
                            if(!(status_value == "Ok"))
                            {
                                serveritynum +=1;
                            }
                            else{
                                healthnum +=1;
                            }
                        }
                    }

                }
            }
        }
    }
    else
    {
        QMessageBox::information(this, "Warning","False Login Information Or Analysis Failed！");
        qDebug()<<"3"<<"handle errors here";
        //QVariant statusCodeV = logserver->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //qDebug( "found error ....code: %d %d\n", statusCodeV.toInt(), (int)logserver->error());
        //qDebug() <<(logserver->errorString());

    }
    logserver->deleteLater();

    QString rate = QString("%1/%2").arg(serveritynum).arg(healthnum);

    //close loading ui
    emit closeLoading();

    return rate;
}

void AddId::on_cancel_clicked()
{
    this->hide();

    emit showWidget();
}

//reply request of net
void AddId::receiveReply(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError){
        //error information in processing
        qDebug()<<"reply error111:"<<reply->errorString();

        //here return so that avoid Abnormal exit
        return;
    }else{
        //request way
        qDebug()<<"operation:"<<reply->operation();
        qDebug()<<"status code:"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    }
    reply->deleteLater();
}

void AddId::receive_widget_refresh(QString id){
    qDebug() << "id = " << id;

    QString token = postToken(getTokenUrl(id));
    qDebug() <<token;


    QString powerstate = (getCriticalMsg(getSystemUrl(id),token))[1];
    QString serveritynum = getSeverityMsg(getLogServiceUrl(id),token);

    //send severitynum to widget to show
    emit send_realTimeData(powerstate,serveritynum);
}

void AddId::receive_widget_addid()
{
    //receive signal of widget for showing addid
    this->move(515,125);
    this->show();
}
