#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H
#include <QLabel>

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QKeyEvent;
QT_END_NAMESPACE

class ImageView : public QLabel
{
    Q_OBJECT
public:
   explicit  ImageView(QWidget *parent =0);

protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *ev);

signals:
    void cursoPosition(int x,int y,int keyType=1);//keyType:1 click,2 press shift+F2,3 press shift+F3,4 press shift+F3

};




#endif // IMAGEVIEW_H
