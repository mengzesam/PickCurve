#include <QtWidgets>
#include "imageview.h"

ImageView::ImageView(QWidget *parent)
    :QLabel(parent)
{

}

void ImageView::mousePressEvent(QMouseEvent *event)
{
    int x=event->pos().x();
    int y=event->pos().y();
    emit  cursoPosition(x,y);
    QWidget::mousePressEvent(event);
}

void ImageView::keyPressEvent(QKeyEvent *ev)
{
    QPoint pos=mapFromGlobal(cursor().pos());
    int x=pos.x();
    int y=pos.y();
    int keyType=0;
    if(Qt::Key_F2==ev->key())
        keyType=2;
    else if(Qt::Key_F3==ev->key())
        keyType=3;
    else if(Qt::Key_F4==ev->key())
        keyType=4;
    if(keyType>=2 && keyType<=4 && x>=0 && y>=0 && x<=width() && y<=height())
        emit  cursoPosition(x,y,keyType);
     QWidget::keyPressEvent(ev);
}
