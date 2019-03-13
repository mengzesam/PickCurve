#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
class QResizeEvent;
class QMouseEvent;
class QFileDialog;
class QScrollArea;
class QImage;
class QTableWidgetItem;
class QPoint;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void open();
    void help();
    void getImagePosition(int x,int y,int keyType=1);
    void tableChanged(QTableWidgetItem* item);
    void saveData();
    void copyData();
    void paste();
    void undo();
    void clearData();

private:
    void SetupUi();
    void SetupMenu();
    void SetupTable();
    void SetupSignal();
    bool loadFile(const QString &fileName);
    int saveFile(const QString &fileName);
    void setImage(const QImage &newImage);
    static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);
    void setBasePosition(int x,int y);
    void setDiagonalPosition(int x,int y);
    void pickCurve(int x,int y);

private:
    Ui::MainWindow *ui;
    bool isImage;
    int readyStatus;
    int currentRow;    
    double scaleFactor;
    double unitX;
    double unitY;
    double baseOffsetX;
    double baseOffsetY;
    int baseX;
    int baseY;
    int XScale;
    int YScale;
    QImage image;


private:
    const static int ROWS=104;
    const static int COLUMNS=3;
};









#endif // MAINWINDOW_H
