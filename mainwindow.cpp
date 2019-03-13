#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageview.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isImage(false),
    readyStatus(0),
    currentRow(3),
    scaleFactor(1.0),
    unitX(50.0),
    unitY(1.1),
    baseOffsetX(-30.0),
    baseOffsetY(-0.4),
    baseX(0),
    baseY(0),
    XScale(1),
    YScale(1)
{
    SetupUi();
    SetupMenu();
    SetupTable();
    SetupSignal();
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(-10+QApplication::desktop()->availableGeometry().width()
                 ,-30+QApplication::desktop()->availableGeometry().height());
    setWindowTitle(tr("拾取曲线上点"));
    setWindowIcon(QPixmap(":/image/pickcurve.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupUi()
{
    ui->setupUi(this);
    ui->gridLayout->setColumnStretch(0,1);
    ui->gridLayout->setColumnStretch(1,4);
    ui->imageLabel->setText("image container");
    ui->imageLabel->setAlignment(Qt::AlignCenter);
    ui->imageLabel->setBackgroundRole(QPalette::Base);
    ui->imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->imageLabel->setScaledContents(true);
}

void MainWindow::SetupMenu()
{
    connect(ui->actionOpen,&QAction::triggered,this,&MainWindow::open);
    connect(ui->actionSavePickData,&QAction::triggered,this,&MainWindow::saveData);
    connect(ui->actionClearData,&QAction::triggered,this,&MainWindow::clearData);
    connect(ui->actionExit,&QAction::triggered,this,&MainWindow::close);
    connect(ui->actionHelp,&QAction::triggered,this,&MainWindow::help);

    ui->actionSavePickData->setEnabled(false);
    ui->actionClearData->setEnabled(false);
}

void MainWindow::SetupTable()
{
    QStringList heads;
    heads<<"名称"<<"X"<<"Y";
    ui->table->setColumnCount(COLUMNS);
    ui->table->setRowCount(ROWS);
    ui->table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    ui->table->verticalHeader()->hide();
    for(int c=0;c<COLUMNS;c++){
        ui->table->setHorizontalHeaderItem(c,
                                           new QTableWidgetItem(heads.at(c)));
    }
    QFont headFont=ui->table->horizontalHeader()->font();
    headFont.setBold(true);
    ui->table->horizontalHeader()->setFont(headFont);
    ui->table->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}");
    QStringList names;
    names<<"一单位倍数"<<"基点偏置"<<"基点位置F4"<<"基点对角位置F3";
    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLUMNS;c++){
            ui->table->setItem(r,c,new QTableWidgetItem(""));
        }
    for(int r=0;r<ROWS;r++)
        ui->table->item(r,0)->setFlags((ui->table->item(r,0)->flags()) ^ (Qt::ItemIsEditable));
    ui->table->item(2,1)->setFlags((ui->table->item(2,1)->flags()) ^ (Qt::ItemIsEditable));
    ui->table->item(3,1)->setFlags((ui->table->item(3,1)->flags()) ^ (Qt::ItemIsEditable));
    ui->table->item(2,2)->setFlags((ui->table->item(2,2)->flags()) ^ (Qt::ItemIsEditable));
    ui->table->item(3,2)->setFlags((ui->table->item(3,2)->flags()) ^ (Qt::ItemIsEditable));    
    ui->table->item(0,1)->setData(Qt::EditRole,tr("%1").arg(unitX));
    ui->table->item(0,2)->setData(Qt::EditRole,tr("%1").arg(unitY));
    ui->table->item(1,1)->setData(Qt::EditRole,tr("%1").arg(baseOffsetX));
    ui->table->item(1,2)->setData(Qt::EditRole,tr("%1").arg(baseOffsetY));
    readyStatus=2;
    for(int r=0;r<4;r++){
        ui->table->item(r,0)->setData(Qt::EditRole,names.at(r));
    }
    ui->table->item(4,0)->setData(Qt::EditRole,tr("point1 F2"));
    for(int r=5;r<ROWS;r++){
        ui->table->item(r,0)->setData(Qt::EditRole,tr("point%1").arg(r-3));
    }
}

void MainWindow::SetupSignal()
{
    connect(ui->imageLabel,&ImageView::cursoPosition,this,&MainWindow::getImagePosition);
    connect(ui->table,&QTableWidget::currentItemChanged,this,&MainWindow::tableChanged);
    connect(ui->table,&QTableWidget::itemChanged,this,&MainWindow::tableChanged);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    int width=event->size().width();
    int table_width=ui->table->size().width();
    if(table_width<width/5)
        table_width=width*21/120;
    else
        table_width=0.90*table_width;
    ui->table->setColumnWidth(0,table_width/2);
    ui->table->setColumnWidth(1,table_width/4);
    ui->table->setColumnWidth(2,table_width/4);
    QWidget::resizeEvent(event);
}


bool MainWindow::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);
    setWindowTitle(tr("拾取曲线上点:%1").arg(fileName));
    return true;
}

void MainWindow::setImage(const QImage &newImage)
{
    image = newImage;
    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;
    ui->actionFit2Window->setEnabled(true);
    //updateActions();
    if (!ui->actionFit2Window->isChecked())
        ui->imageLabel->adjustSize();
}

void MainWindow::initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

void MainWindow::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted) {
        isImage=loadFile(dialog.selectedFiles().first());
        if(isImage){
            return;
        }
    }

}

void MainWindow::help()
{
    QMessageBox::about(this, tr("拾取曲线上的点以拟合曲线"),
            tr("<p>先加载曲线图片</p>"
               "<p>设置单位倍数</p>"
               "<p>设置基点的坐标值</p>"
               "<p>拾取前移动鼠标到图片上任一位置鼠标点击一下以使输入处于激活状态</p>"
               "<p>移动鼠标到曲线基点位置，按F4或Shift+F4</p>"
               "<p>移动鼠标到曲线基点对角点位置，按F3或Shift+F3</p>"
               "<p>移动鼠标到需要拾取点上，按F2或Shift+F2</p>"));
}

void MainWindow::getImagePosition(int x, int y,int keyType)
{
    ui->imageLabel->setFocus();
    ui->statusBar->showMessage(tr("x:%1,y:%2").arg(x).arg(y));
    if(keyType==4)
        setBasePosition(x,y);
    else if(keyType==3)
        setDiagonalPosition(x,y);
    else if(keyType==2)
        pickCurve(x,y);
}

void MainWindow::tableChanged(QTableWidgetItem *item)
{
    int r=item->row();
    int c=item->column();
    if(r<=1 && c>=1){
        QVariant xs=ui->table->item(0,1)->data(Qt::DisplayRole);
        QVariant ys=ui->table->item(0,2)->data(Qt::DisplayRole);
        if((!xs.isNull()) && (!ys.isNull())){
            unitX=(xs.toDouble());
            unitY=(ys.toDouble());
            readyStatus=1;
        }
        xs=ui->table->item(1,1)->data(Qt::DisplayRole);
        ys=ui->table->item(1,2)->data(Qt::DisplayRole);
        if((!xs.isNull()) && (!ys.isNull()) && readyStatus==1){
            baseOffsetX=(xs.toDouble());
            baseOffsetY=(ys.toDouble());
            readyStatus=2;
        }
        currentRow=3;
    }
}

void MainWindow::setBasePosition(int x, int y)
{
    if(!isImage){
        ui->statusBar->showMessage(tr("未加载曲线图片！"));
        return ;
    }
    if(readyStatus<1){
        ui->statusBar->showMessage(tr("单位倍数未设置"));
        return ;
    }
    if(readyStatus<2){
        ui->statusBar->showMessage(tr("基点偏置未设置"));
        return ;
    }
    ui->table->item(2,1)->setData(Qt::EditRole,tr("%1").arg(x));
    ui->table->item(2,2)->setData(Qt::EditRole,tr("%1").arg(y));
    baseX=x;
    baseY=y;
    readyStatus=3;
    currentRow=3;
}

void MainWindow::setDiagonalPosition(int x, int y)
{
    if(!isImage){
        ui->statusBar->showMessage(tr("未加载曲线图片！"));
        return ;
    }
    if(readyStatus<1){
        ui->statusBar->showMessage(tr("单位倍数未设置"));
        return ;
    }
    if(readyStatus<2){
        ui->statusBar->showMessage(tr("基点偏置未设置"));
        return ;
    }
    if(readyStatus<3){
        ui->statusBar->showMessage(tr("基点位置未设置"));
        return ;
    }
    ui->table->item(3,1)->setData(Qt::EditRole,tr("%1").arg(x));
    ui->table->item(3,2)->setData(Qt::EditRole,tr("%1").arg(y));
    if(x==baseX){
        ui->statusBar->showMessage(tr("基点与对角点X值相同！"));
        return ;
    }
    if(y==baseY){
        ui->statusBar->showMessage(tr("基点与对角点Y值相同！"));
        return ;
    }
    XScale=x-baseX;
    YScale=y-baseY;
    readyStatus=4;
    currentRow=3;
}

void MainWindow::pickCurve(int x, int y)
{
    if(!isImage){
        ui->statusBar->showMessage(tr("未加载曲线图片！"));
        return ;
    }
    if(readyStatus<1){
        ui->statusBar->showMessage(tr("单位倍数未设置"));
        return ;
    }
    if(readyStatus<2){
        ui->statusBar->showMessage(tr("基点偏置未设置"));
        return ;
    }
    if(readyStatus<3){
        ui->statusBar->showMessage(tr("基点位置未设置"));
        return ;
    }
    if(readyStatus<4){
        ui->statusBar->showMessage(tr("基点对角位置未设置"));
        return ;
    }
    currentRow++;
    ui->actionSavePickData->setEnabled(true);
    ui->actionClearData->setEnabled(true);
    double xx=baseOffsetX+unitX*(x-baseX)/XScale;
    double yy=baseOffsetY+unitY*(y-baseY)/YScale;
    int r=currentRow<ROWS?currentRow:ROWS-1;
    ui->table->item(r,1)->setData(Qt::EditRole,tr("%1").arg(xx));
    ui->table->item(r,2)->setData(Qt::EditRole,tr("%1").arg(yy));
}

int MainWindow::saveFile(const QString &fileName)//return 0 success,1 no data for save, 2 cannot create file
{
    if(currentRow<4)
        return 1;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return 2;
    QTextStream out(&file);
    QVariant x,y;
    for(int r=4;r<=currentRow;r++){
        x=ui->table->item(r,1)->data(Qt::DisplayRole);
        y=ui->table->item(r,2)->data(Qt::DisplayRole);
        out <<tr("point%1,").arg(r-3)<<tr("%1,").arg(x.toString())<<tr("%1\n").arg(y.toString());
    }
    return 0;
}

void MainWindow::saveData()
{
    if(!isImage)
        return;
    if(currentRow<4)
        return;
    QFileDialog dialog(this, tr("Save File"));
    static bool firstDialog = true;
    if (firstDialog) {
        firstDialog = false;
        const QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        dialog.setDirectory(dataLocations.isEmpty() ? QDir::currentPath() : dataLocations.last());
    }
    QStringList filters;
    filters<<"CSV files (*.csv)"<<"Text files (*.txt *.dat)"<<"Any files (*.*)";
    dialog.setNameFilters(filters);
    dialog.setDefaultSuffix("csv");
    while(dialog.exec()==QDialog::Accepted){
        int saveRet=saveFile(dialog.selectedFiles().first());
        if(saveRet==0)
            return;
        if(saveRet==2){
             QMessageBox::warning(this,tr("save file warnning"),
                                  tr("cannot create save file!"));
             return;
        }
        else if(saveRet==1){
            QMessageBox::warning(this,tr("save file warnning"),
                                 tr("no data for saving!"));
            return;
        }

    }
}

void MainWindow::copyData()
{

}

void MainWindow::paste()
{

}

void MainWindow::undo()
{

}

void MainWindow::clearData()
{
    ui->table->item(2,1)->setData(Qt::EditRole,"");
    ui->table->item(2,2)->setData(Qt::EditRole,"");
    ui->table->item(3,1)->setData(Qt::EditRole,"");
    ui->table->item(3,2)->setData(Qt::EditRole,"");
    for(int r=4;r<=currentRow;r++){
        ui->table->item(r,1)->setData(Qt::EditRole,"");
        ui->table->item(r,2)->setData(Qt::EditRole,"");
    }
    currentRow=3;
    readyStatus=2;
    ui->actionClearData->setEnabled(false);
    ui->actionSavePickData->setEnabled(false);
}
