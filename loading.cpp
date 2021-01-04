#include "loading.h"
#include "ui_loading.h"

Loading::Loading(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Loading)
{
    ui->setupUi(this);

    //init loading ui
    initLoadingUi();
}

Loading::~Loading()
{
    delete ui;
}

void Loading::paintEvent(QPaintEvent *){
    //set window background
    QPainter p;
    p.begin(this);
    p.drawPixmap(rect(),QPixmap(":/skin/21"));
}

void Loading::initLoadingUi(){

    //set icon,name and size for window
    setWindowTitle(tr("奔跑中.."));
    setWindowIcon(QIcon(":/main/servers"));
    setMouseTracking(true);
    setMaximumSize(QSize(265,162));
}

void Loading::receive_addid_loading(){
    this->show();
}

void Loading::receive_addid_closeloading(){
    this->close();
}
