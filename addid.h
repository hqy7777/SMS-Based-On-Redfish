#ifndef ADDID_H
#define ADDID_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <typeinfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMessageBox>
#include <QEventLoop>

namespace Ui {
class AddId;
}

class AddId : public QWidget
{
    Q_OBJECT

public:
    explicit AddId(QWidget *parent = nullptr);
    ~AddId();

    void paintEvent(QPaintEvent *);

    void initAddidUi();

    QString getTokenUrl(QString url);

    QString getSystemUrl(QString url);

    QString postToken(QString id);

    QString analysisSessionsJson(QNetworkReply *reply);

    QStringList getCriticalMsg(QString url,QString token);

    QStringList analysisSystemJson(QNetworkReply *sys);

    QString getLogServiceUrl(QString url);

    QString getSeverityMsg(QString url, QString token);

    QString analysisLogServerJson(QNetworkReply *sys);

    void receiveReply(QNetworkReply *reply);

    QStringList readId();

    //private for this class itself
    void connectDatabase();

private slots:

    void receive_widget_addid();

    void on_save_clicked();

    void on_cancel_clicked();

    void receive_widget_refresh(QString id);

signals:

    void showWidget();

    void showLabel(QString,QString,QString,QString,QString,QString);

    void showLoading();

    void closeLoading();

    void send_realTimeData(QString powerstate,QString severitynum);

private:
    Ui::AddId *ui;
    QNetworkAccessManager *manager;

};
#endif // ADDID_H
