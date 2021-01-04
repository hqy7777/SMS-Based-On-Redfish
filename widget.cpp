#include "widget.h"
#include "ui_widget.h"
#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    initWidgetUi();

    //read database for the treewidget data
    readRealData();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initWidgetUi(){

    //set icon,name and size for window
    setWindowTitle(tr("服务器小助手"));
    setWindowIcon(QIcon(":/main/servers"));
    setMouseTracking(true);
    setMaximumSize(QSize(598,420));

    //set tip message
    ui->help->setToolTip("帮助");
    ui->addid->setToolTip("添加服务器");
    ui->init->setToolTip("删除所有服务器");
}

void Widget::readRealData(){

    createDatabase();

    //read data
    QSqlQuery sql_query;

    sql_query.exec("select * from realdata");
    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
    }
    else
    {
        qDebug()<<"表：";
        //read database
        while(sql_query.next())
        {
            QString id = sql_query.value(0).toString();
            QString manufacturer= sql_query.value(1).toString();
            QString powerstate = sql_query.value(2).toString();
            QString serveritynum = sql_query.value(3).toString();
            QString uuid = sql_query.value(4).toString();
            QString health = sql_query.value(5).toString();
            qDebug()<<QString("id:%1    manufacturer:%2    powerstate:%3   serveritynum:%4   uuid:%5  health:%6").arg(id).arg(manufacturer).arg(powerstate).arg(serveritynum).arg(uuid).arg(health);
            QStringList strs;

            //
            strs<<QString(id)<<QString(manufacturer)<<QString(powerstate)<<QString(serveritynum)<<QString(uuid);

            //add one line for treewidget,content is strs
            QTreeWidgetItem *strsroot = new QTreeWidgetItem(ui->treeWidget,strs);

            //set the header to appropriate size
            QHeaderView *head=ui->treeWidget->header();
            head->setSectionResizeMode(QHeaderView::ResizeToContents);

            ui->treeWidget->setCurrentItem(strsroot);//set currentitem

            setItemStyle(sql_query.value(5).toString(),strsroot);            
        }
        qDebug()<<"成功显示在界面上.";
    }
    //item menu
    createItemMenu();
}

void Widget::createDatabase(){
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
        qDebug() << "Error: Failed to connect database." << database.lastError();
    }
    else
    {
        qDebug() <<"数据库连接成功！"<<"从数据库读取数据.";
    }
}

void Widget::setItemStyle(QString health,QTreeWidgetItem *strsroot){

    //tip
    strsroot->setToolTip(0,"服务器ID");
    strsroot->setToolTip(1,"产品名称");
    strsroot->setToolTip(2,"开关状态");
    strsroot->setToolTip(3,"警告及严重信息占比");
    strsroot->setToolTip(4,"通用唯一识别码");

    //set color of item
    if(health == "OK"){
        for(int i=0;i<5;i++){
          strsroot->setBackground(i,QColor(0,170,0));
        }
    }
    else{
        strsroot->setBackground(0,QBrush(QColor(255,0,0)));
        for(int i=0;i<5;i++){
          strsroot->setBackground(i,QColor(255,0,0));
        }
    }

    //set currentItem to toplevel
    ui->treeWidget->addTopLevelItem(strsroot);
}

void Widget::createItemMenu(){

    m_currentItem = new QMenu(this);

    m_deleteItem = new QAction(tr("删除此服务器"),this);

    m_serverityFresh = new QAction(tr("刷新告警信息"),this);

    connect(m_currentItem,SIGNAL(triggered(QAction *)),this,SLOT(onMenuTriggered(QAction *)));

}

//reconstruct contextMenuEvent function which can identify the mouse event
void Widget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (item != NULL){
        m_currentItem->addAction(m_serverityFresh);
        m_currentItem->addAction(m_deleteItem);
        m_currentItem->exec(QCursor::pos());
    }
    event->accept();
}

//treewidget action function
void Widget::onMenuTriggered(QAction *action)
{
    if (action ==m_serverityFresh)
    {
        //refresh real data
        refresh();
    }
    else if (action == m_deleteItem)
    {
         deleteServer();
    }
}

//refresh value of powerstatus and serveritynum
void Widget::refresh()
{
    int choose;
    choose= QMessageBox::question(this, tr("刷新"),
                                   QString(tr("小福需要短暂的加载,"
                                        "确认刷新ID为%1的开关机状态及严重告警占比数量？").arg(ui->treeWidget->currentItem()->text(0))),
                                   QMessageBox::Yes | QMessageBox::No);

    if (choose== QMessageBox::No)
    {
        return;
    }
    else if (choose== QMessageBox::Yes)
    {
        emit refresh_realtimedata(ui->treeWidget->currentItem()->text(0));
    }
}

//delete one line of treewidget
void Widget::deleteServer()
{
    //delete this server
    int choose;
    choose= QMessageBox::question(this, tr("删除"),
                                   QString(tr("确认删除ID为%1的服务器?").arg(ui->treeWidget->currentItem()->text(0))),
                                   QMessageBox::Yes | QMessageBox::No);

    if (choose== QMessageBox::No)
    {
        return;
    }
    else if (choose== QMessageBox::Yes)
    {
        createDatabase();
        QTreeWidgetItem *item = ui->treeWidget->currentItem();
        //delete the data of the database——realdata
        QSqlQuery sql_query;
        if(!sql_query.exec(QString("delete from realdata where id = '%1'").arg(item->text(0))))
        {
            qDebug() << "delete data from realdata";
            qDebug()<<sql_query.lastError();
        }
        else
        {
            qDebug()<<"deleted!";
        }
        delete(item);
    }

}

void Widget::paintEvent(QPaintEvent *){
    //set window background
    QPainter p;
    p.begin(this);
    p.drawPixmap(rect(),QPixmap(":/skin/21"));
}

void Widget::receive_save(QString id,QString manufacturer,QString powerstate,QString severity,QString uuid,QString health)
{
      this->show();
      QStringList strs;
      QStringList str1;
      strs<<QString("id")<<QString("manufacturer")<<QString(powerstate)<<QString(severity)<<QString("uuid");


      //set the header to appropriate size
      QHeaderView *head=ui->treeWidget->header();
      head->setSectionResizeMode(QHeaderView::ResizeToContents);

      //add one line for treewidget,content is strs
      QTreeWidgetItem *strsroot = new QTreeWidgetItem(ui->treeWidget,strs);

      //set currentitem
      ui->treeWidget->setCurrentItem(strsroot);

      //item sytle
      setItemStyle(health,strsroot);

      //item menu
      createItemMenu();

      //save lineEdit record to Database
      writeDatabase(health);
}

void Widget::writeDatabase(QString health)
{
    //create and connect database
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

    //create table
    QSqlQuery sql_query;//set id text primary key

    //judge table exist or not
    bool isTableExist = \
        sql_query.exec(QString("select count(*) from realdata where type='table' and name='%1'").arg(("realdata")));
    if(!isTableExist){
        if(!sql_query.exec("create table realdata(id text,manufacturer text,powerstate text,serveritynum text,uuid text,health text)"))
        {
            qDebug() << "Error: Fail to create table."<< sql_query.lastError();
        }
        else
        {
            qDebug() << "Table created!";
        }
    }
    else{
    }

    //ergodic the treewidget item,insert the data into database
    QTreeWidgetItemIterator it(ui->treeWidget->currentItem());
    while(*it){
        (*it)->setHidden(false);
        QTreeWidgetItem *item = *it;
        if(!sql_query.exec(QString("INSERT INTO realdata VALUES('%1','%2','%3','%4','%5','%6')").arg(item->text(0)).arg(item->text(1)).arg(item->text(2)).arg(item->text(3)).arg(item->text(4)).arg(health)))
        {
            qDebug() << "insert !!" <<sql_query.lastError();
        }
        else
        {
            qDebug() << "inserted FirstItemData!";
        }
        ++it;
    }

    //close database
    database.close();
}

void Widget::deleteRealData(){

    //create and connect database
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

    //delete the table
    QSqlQuery sql_query;
    sql_query.exec("drop table realdata");
    if(sql_query.exec())
    {
        qDebug() << sql_query.lastError();
    }
    else
    {
        qDebug() << "table cleared";
    }
}

void Widget::receive_addid_cancel()
{
    this->show();
}

// receive value of powerstatus and serveritynum from addid
void Widget::receive_realTimeData(QString powerstate,QString serverity){

    ui->treeWidget->currentItem()->setText(2, powerstate);
    ui->treeWidget->currentItem()->setText(3, serverity);
    QMessageBox::about(NULL, "刷新", "成功刷新开关机状态及严重告警占比");
}

void Widget::on_init_clicked()
{
    //judge if the treewidget exists
    if(ui->treeWidget->currentItem()){
        int choose;
        choose= QMessageBox::question(this, tr("初始化"),
                                       QString(tr("确认删除所有服务器?")),
                                       QMessageBox::Yes | QMessageBox::No);
        if (choose== QMessageBox::No)
        {
            return;
        }
        else if (choose== QMessageBox::Yes)
        {
            //delete the table of database
            deleteRealData();

            //delete the treewidget for ui
            ui->treeWidget->clear();
        }
    }
    else{
        QMessageBox::about(NULL, "删除", "当前已无可删除的服务器");
    }
}

void Widget::on_help_clicked()
{
    QMessageBox::about(NULL, "帮助", "小福可支持添加新服务器，删除服务器等功能，快去试试吧~");
}

void Widget::on_addid_clicked()
{
    this->hide();
    emit showAddId();
}

