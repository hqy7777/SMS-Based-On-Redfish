#include "widget.h"
#include "addid.h"
#include "loading.h"
#include <QApplication>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget *w = new Widget;
    AddId *i = new AddId;
    Loading *l = new Loading;
    w->show();

    //all connections

    /* + function
    ui from widget to addid for adding servers
    */
    QObject::connect(w,SIGNAL(showAddId()),i,SLOT(receive_widget_addid()));
    /* save function
    ui from addid to widget for showing servers data
    */
    QObject::connect(i,SIGNAL(showLabel(QString,QString,QString,QString,QString,QString)),w,SLOT(receive_save(QString,QString,QString,QString,QString,QString)));
    /* cancel function
    ui from addid to widget for showing widget
    */
    QObject::connect(i,SIGNAL(showWidget()),w,SLOT(receive_addid_cancel()));
    /* show loading ui
    ui from addid to loading for showing waiting process
    */
    QObject::connect(i,SIGNAL(showLoading()),l,SLOT(receive_addid_loading()));
    /* close loading ui
    ui from addid to loading for showing widget ui
    */
    QObject::connect(i,SIGNAL(closeLoading()),l,SLOT(receive_addid_closeloading()));
    /* refresh realdata
    ui from widget to addid for refreshing realdata——powerstate and serveritynum
    */
    QObject::connect(w,SIGNAL(refresh_realtimedata(QString)),i,SLOT(receive_widget_refresh(QString)));
    /* refresh realdata
    ui from addid to widget for sending realdata——powerstate and serveritynum
    */
    QObject::connect(i,SIGNAL(send_realTimeData(QString,QString)),w,SLOT(receive_realTimeData(QString,QString)));
    return a.exec();
}
