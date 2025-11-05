#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<iostream>

#include <QAbstractButton>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    status = new QPushButton(this);
    lcd=new QLCDNumber(this);//计时器
    lcdm=new QLCDNumber(this);
    runTimer=new QTimer(this);

    status->setParent(this);
    status->resize(40,40);
    status->setEnabled(true);
    status->setCheckable(true);
    status->raise();

    lcd->setParent(this);
    lcd->resize(80,40);
    lcd->setDigitCount(5);
    lcd->setSegmentStyle(QLCDNumber::Flat);

    lcdm->setParent(this);
    lcdm->resize(80,40);
    lcdm->setDigitCount(4);
    lcdm->setSegmentStyle(QLCDNumber::Flat);

    runTimer->setParent(this);

    //将各模式连接到槽函数
    connect(ui->actionEasy,SIGNAL(triggered()),this, SLOT(slot_easy()));
    connect(ui->actionMedium,SIGNAL(triggered()),this, SLOT(slot_medium()));
    connect(ui->actionHard,SIGNAL(triggered()),this, SLOT(slot_hard()));
    connect(status, SIGNAL(clicked()), this, SLOT(slot_newGame()));
    connect(runTimer, SIGNAL(timeout()), this, SLOT(slot_update()));

    flagImage=QPixmap(":/new/prefix1/image/flag.png");
    mineImage=QPixmap(":/new/prefix1/image/explosion.png");
    smileImage=QPixmap(":/new/prefix1/image/smile.png");
    failImage=QPixmap(":/new/prefix1/image/fail.png");
    successImage=QPixmap(":/new/prefix1/image/success.png");
    rightImage=QPixmap(":/new/prefix1/image/tada.png");
    rightImage=rightImage.scaled(256, 256);

    rows = 9;
    cols = 9;
    mines = 10;
    newGame();

}

void MainWindow::initGame()
{
    //随机初始化 雷
        m.clear();
        for(int i = 0; i<mines; i++)
        {
            qsrand(QTime::currentTime().msec());
            int x = qrand()%cols;
            int y = qrand()%rows;

            while(m.contains(QPoint(x,y)))
            {//当已经包含了这个雷 则重新随机生成一个雷
                x = qrand()%cols;
                y = qrand()%rows;
            }
            m.append(QPoint(x,y));
        }
        //建立2维数组保存所有元素位置，方便索引
        for(int i=0; i<cols; i++) //列
        {
            QVector<Grid*> rowItems;//Item对象指针的容器
            for(int j=0; j<rows; j++)
            {
                QPoint pos = QPoint(i,j);
                Grid* pItem = new Grid(pos);
                if(m.contains(pos))   //若该位置是雷
                {
                    pItem->isMine = true;
                }
                rowItems.append(pItem);
            }
            grids.append(rowItems);
        }
        //计算格子的数字
        for(int i=0; i<cols; i++)
        {
            for(int j=0; j<rows; j++)
            {
                if (grids[i][j]->isMine)
                {
                    continue;
                }
                int nCountMines = 0;
                //求每个点附近的8个点的是雷的总数
                for (int m=-1;m<=1;m++)
                {
                    for (int n=-1; n<=1;n++)
                    {
                        if (m==0 && n==0)
                        {
                            continue;
                        }
                        QPoint ptNew = QPoint(i+m,j+n);
                        if (!pointInGameArea(ptNew))
                        {
                            continue;
                        }

                        if (grids[i+m][j+n]->isMine)
                        {
                            nCountMines++;
                        }
                    }
                }
                grids[i][j]->number = nCountMines;
            }
        }
}
void MainWindow::newGame()
{
    gameOver = false; //开始游戏没有失败
    gameSuccess=false;
    seconds=0;
    mleft=mines;
    //初始化界面大小
    resize(START_X*2 + cols*RECT_WIDTH  ,START_Y*2 + rows*RECT_HEIGHT);
    //设置状态按钮
    status->move((START_X*2 + cols*RECT_WIDTH)/2,35);
    status->setIcon(QIcon(smileImage));

    lcd->move(3*(START_X*2 + cols*RECT_WIDTH)/4,35);
    lcd->display("0");
    runTimer->stop();

    lcdm->move((START_X*2 + cols*RECT_WIDTH)/4,35);
    lcdm->display(mleft);

    releaseGame();
    initGame();
}
//游戏结束
void MainWindow::overGame()
{
    gameOver = true;
    runTimer->stop();
    for (int i=0; i<grids.size(); i++)
    {
        for (int j=0;j<grids[i].size(); j++)
        {
            if (grids[i][j]->isMine)
            {
                grids[i][j]->isMarked = true; //是雷，显示
            }
            else
            {
                grids[i][j]->isMarked = false; //非雷，打开
                grids[i][j]->isOpen = true;
            }
        }
    }
}
void MainWindow::releaseGame()
{   //清除所有的指针
    for (int i=0; i<grids.size(); i++)
    {
        for (int j=0;j<grids[i].size(); j++)
        {
            if (grids[i][j] != nullptr)
            {
                delete grids[i][j];
                grids[i][j] = nullptr;
            }
        }
    }
    grids.clear();
}
bool MainWindow::findAll()
{
    bool bFindAll = true;
    //遍历二维数组
    for (int i=0; i<grids.size(); i++)
    {
        for (int j=0;j<grids[i].size(); j++)
        {
            //只要存在一个雷没被标记，都不算找完
            Grid* pItem = grids[i][j];
            if (pItem->isMine)
            {
                if (!pItem->isMarked)
                {
                    bFindAll = false;
                }
            }
//            else
//            {
//                if (!pItem->isOpen)
//                {
//                    bFindAll = false;
//                }
//            }
        }
    }
    return bFindAll;
}
void MainWindow::paintEvent(QPaintEvent *)
{
    drawBoard(); //画面板
    drawItems(); //画网格

    //刷新游戏画面
    update();
}
void MainWindow::drawBoard()
{
    QPainter painter(this);
    painter.setBrush(Qt::white);
    painter.drawRect( 0,0,width(),height());
}

void MainWindow::drawItems()
{
    QPainter painter(this);
    painter.setBrush(Qt::lightGray);
    painter.setPen(QPen(QColor(Qt::black),2));

    for(int i=0; i<cols; i++)
    {
        for(int j=0; j<rows; j++)
        {
            drawItem(painter,grids[i][j]);
        }
    }
}

void MainWindow::drawItem(QPainter &painter, Grid *pItem)
{
    if(pItem->isMarked)  //标记为雷
    {
        if (gameOver)
        {
            if(gameSuccess)
            {
                status->setIcon(QIcon(successImage));
                QRect rcSrc(0,0,flagImage.width(),flagImage.height());
                QRect rcTarget(START_X + pItem->m_pos.x()*RECT_WIDTH + 2,START_Y + pItem->m_pos.y()*RECT_HEIGHT + 2,RECT_WIDTH-4,RECT_HEIGHT-4);
                painter.drawPixmap(rcTarget,rightImage,rcSrc);
            }
            else
            {
                //游戏结束，显示为雷
                //if(pItem->isMine)
                status->setIcon(QIcon(failImage));
                QRect rcSrc(0,0,flagImage.width(),flagImage.height());
                QRect rcTarget(START_X + pItem->m_pos.x()*RECT_WIDTH + 2,START_Y + pItem->m_pos.y()*RECT_HEIGHT + 2,RECT_WIDTH-4,RECT_HEIGHT-4);
                painter.drawPixmap(rcTarget,mineImage,rcSrc);
            }
        }
        else
        {
            //游戏没结束，显示为旗子
            QRect rcSrc(0,0,flagImage.width(),flagImage.height());
            QRect rcTarget(START_X + pItem->m_pos.x()*RECT_WIDTH + 2,START_Y + pItem->m_pos.y()*RECT_HEIGHT + 2,RECT_WIDTH-4,RECT_HEIGHT-4);
            painter.drawPixmap(rcTarget,flagImage,rcSrc);
        }
        painter.setBrush(Qt::transparent);
        painter.drawRect( START_X + pItem->m_pos.x()*RECT_WIDTH,START_Y + pItem->m_pos.y()*RECT_HEIGHT,RECT_WIDTH,RECT_HEIGHT);
        return;
    }
    else if (pItem->isOpen)
    {
        if(pItem->number == 0)
            //如果为0 则为白色
        {
            painter.setBrush(Qt::white);
        }
        else  //显示数字
        {
            QFont font;
            font.setPointSize(10);
            font.setFamily(("SimHei"));
            font.setBold(true);

            painter.setBrush(Qt::white);
            painter.drawRect( START_X + pItem->m_pos.x()*RECT_WIDTH,
                              START_Y + pItem->m_pos.y()*RECT_HEIGHT,RECT_WIDTH,RECT_HEIGHT);

            //painter.setBrush(Qt::white);
            painter.setFont(font);
            painter.drawText( START_X + pItem->m_pos.x()*RECT_WIDTH  + 8,
                              START_Y + pItem->m_pos.y()*RECT_HEIGHT + 22,QString("%1").arg(pItem->number));
            return;
        }
    }
    else
    {
        painter.setBrush(Qt::lightGray);
    }
    painter.drawRect( START_X + pItem->m_pos.x()*RECT_WIDTH,START_Y + pItem->m_pos.y()*RECT_HEIGHT,RECT_WIDTH,RECT_HEIGHT);

}

//运气好时点到一个空白元素，可能打开挨着的一大片
void MainWindow::openEmptyItem(QPoint pt)
{
    //对于空白元素，有上下左右4个方向挨着的空白元素，就打开并继续查找空白元素
    QVector<QPoint> directions;
    directions.push_back(QPoint(-1,0));
    directions.push_back(QPoint(1,0));
    directions.push_back(QPoint(0,-1));
    directions.push_back(QPoint(0,1));
    for (int i=0; i<directions.size(); i++)
    {
        QPoint ptNew = pt + directions[i];
        if (!pointInGameArea(ptNew))
        {
            continue;
        }
        Grid* pItem = grids[ptNew.x()][ptNew.y()];
        if (!pItem->isMine && !pItem->isOpen && !pItem->isMarked && pItem->number >= 0)
        {
            //若不是雷 未被打开 未被标记 且数大于等于0
            //则将其打开
            pItem->isOpen = true;

            //对于找到的空白元素，在它的8个方向上有数字元素就打开
            QVector<QPoint> directions2 = directions;
            directions2.push_back(QPoint(-1,-1));
            directions2.push_back(QPoint(1,1));
            directions2.push_back(QPoint(1,-1));
            directions2.push_back(QPoint(-1,1));
            for (int j=0; j<directions2.size(); j++)
            {
                //对四个方向进行递归查看
                QPoint ptNew2 = ptNew + directions2[j];
                if(!pointInGameArea(ptNew2))
                {
                    continue;
                }
                Grid* pItem2 = grids[ptNew2.x()][ptNew2.y()];
                if (!pItem2->isMine && !pItem2->isOpen && !pItem2->isMarked && pItem2->number > 0)
                {
                    //若不是雷 未被打开 未被标记 且数大于0
                    //则将其打开
                    pItem2->isOpen = true;
                }
            }
            //递归查找上下左右4个方向的空白元素
            openEmptyItem(ptNew);
        }
    }
}

bool MainWindow::pointInGameArea(QPoint pt)
{
    if(pt.x()>=0 && pt.x()< cols && pt.y()>=0 && pt.y()< rows)
    {
        return true;
    }
    return false;
}
void MainWindow::mousePressEvent(QMouseEvent *e)
{
    //得到鼠标处的格子坐标
    QPoint pt;
    pt.setX( (e->pos().x() - START_X ) / RECT_WIDTH);
    pt.setY( (e->pos().y() - START_X ) / RECT_HEIGHT);
    //是否点在游戏区域内
    if (!pointInGameArea(pt))
    {
        return;
    }
    //开始计时
    runTimer->start(1000);
    slot_update(); //立即更新

    //获取所点击矩形元素
    Grid* pItem = grids[pt.x()][pt.y()];
    //左键打开元素，右键插旗帜标记
    if(e->button()==Qt::LeftButton)
    {
        //不是已标记的或已打开的空白点，也就是未处理的
        if(!pItem->isMarked && !pItem->isOpen)
        {
            //如果是雷，就GAME OVER
            if (pItem->isMine)
            {
                overGame();
                //runTimer->stop();
                return;
            }
            else
            {
                //打开
                pItem->isOpen = true;
                if (pItem->number == 0)
                {
                    //如果数字是0，也就是不含任何相邻雷的元素，那么递归打开所有的相邻数字是0的元素
                    //也就是点到一个空白处，一下打开一大片的效果
                    openEmptyItem(pt);
                }
                //如果已找到所有雷
                if (findAll())
                {
                    gameSuccess=findAll();
                    overGame();
                     //成功并停止计时

                    return;
                }
            }
        }
    }
    else if(e->button()==Qt::RightButton)
    {
        //已标记过的，取消标记
        if (pItem->isMarked)
        {
            pItem->isMarked = false;
            if(pItem->isMine)
            {
                mleft++;
                lcdm->display(mleft);
            }
        }
        else if (!pItem->isOpen)
        {
            //没标记也没打开，就是未处理的，就插旗帜标记上
            pItem->isMarked = true;
            if(pItem->isMine)
            {
                mleft--;
                lcdm->display(mleft);
            }
            if (findAll())
            {
                gameSuccess=findAll();
                overGame();

                return;
            }
        }
    }
}
void::MainWindow::slot_update()
{
    seconds++;
    int minutes = seconds / 60;
    int secs = seconds % 60;

    lcd->display(QString("%1:%2")
                       .arg(minutes, 2, 10, QChar('0'))
                       .arg(secs, 2, 10, QChar('0')));
}
void::MainWindow::slot_newGame() //在任何时候，重新开始
{
    newGame();
}
void MainWindow::slot_easy()
{
    rows=9;
    cols=9;
    mines=10;
    newGame();
}
void MainWindow::slot_medium()
{
    rows=16;
    cols=16;
    mines=40;
    newGame();
}
void MainWindow::slot_hard()
{
    rows=16;
    cols=30;
    mines=99;
    newGame();
}
MainWindow::~MainWindow()
{
    delete ui;
}

