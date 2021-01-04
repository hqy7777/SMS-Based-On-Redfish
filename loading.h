#ifndef LOADING_H
#define LOADING_H

#include <QWidget>
#include <QPainter>
#include <QIcon>
namespace Ui {
class Loading;
}

class Loading : public QWidget
{
    Q_OBJECT

public:
    explicit Loading(QWidget *parent = nullptr);
    ~Loading();

    void paintEvent(QPaintEvent *);

    void initLoadingUi();


private slots:

    void receive_addid_loading();

    void receive_addid_closeloading();

private:
    Ui::Loading *ui;
};

#endif // LOADING_H
