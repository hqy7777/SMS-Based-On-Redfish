#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QNetworkReply>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMouseEvent>
#include <QDialog>
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QSettings>
#include <QMenu>
#include <QSqlTableModel>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void paintEvent(QPaintEvent *);

    void initWidgetUi();

    void deleteServer();

    QMenu* popMenu;

    QTreeWidgetItem* curItem;

    void createItemMenu();

    void contextMenuEvent(QContextMenuEvent *event);

    void setItemStyle(QString health,QTreeWidgetItem *strsroot);

    void refresh();

    void writeDatabase(QString health);

    void createDatabase();

    void readRealData();

    void deleteRealData();

private slots:

    void on_addid_clicked();

    void receive_save(QString id,QString manufacturer,QString powerstate,QString severity,QString uuid,QString health);

    void receive_addid_cancel();

    void onMenuTriggered(QAction *action);

    void receive_realTimeData(QString powerstate,QString serverity);

    void on_init_clicked();

    void on_help_clicked();

signals:
    void showAddId();

    void refresh_realtimedata(QString id);

private:
    Ui::Widget *ui;

    //menu action
    QAction *m_deleteItem;
    QAction *m_serverityFresh;
    QMenu *m_currentItem;

    QStringList *m_itemData;

    QNetworkAccessManager *manager;
};
#endif // WIDGET_H
