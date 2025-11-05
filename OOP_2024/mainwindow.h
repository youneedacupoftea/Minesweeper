#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "grid.h"
#include <QMainWindow>
#include <QPaintEvent>
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QTime>
#include<QTimer>
#include<QLCDNumber>

//宏定义各种参数数值
#define RECT_WIDTH      35//正方形宽
#define RECT_HEIGHT     35//正方形高

#define START_X         100//起始位置x
#define START_Y         100//起始位置y

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //初始化游戏
    void initGame();
    //新一句游戏
    void newGame();
    //游戏失败，显示所有雷
    void overGame();
    //判断坐标是否在游戏内
    bool pointInGameArea(QPoint pt);
    //结束游戏后释放空间
    void releaseGame();

    //是否找到所有的雷
    bool findAll();
    //绘制图形
    void paintEvent(QPaintEvent *);
    //鼠标操作
    void mousePressEvent(QMouseEvent *e);
    //绘制整个布局
    void drawBoard();
    //绘制物体
    void drawItems();
    //绘制单个物体
    void drawItem(QPainter &painter,Grid * pItem);
    //点击空白元素时，递归查找相邻的空白元素，以及空白元素附近的数字元素(数字是雷数)
    void openEmptyItem(QPoint pt);

public slots:
    void slot_easy();
    void slot_medium();
    void slot_hard();
    void slot_newGame();
    void slot_update();


private:
    Ui::MainWindow *ui;
    int rows;  //行数
    int cols;
    int mines; //雷的数目
    QVector<QPoint> m; //所有雷的位置
    QVector<QVector<Grid*>> grids; //所有网格

    QPushButton* status; //状态按钮
    QLCDNumber *lcd;//计时器的，lcd显示器
    QLCDNumber *lcdm; //剩余雷数的显示器
    QTimer* runTimer; //计时器

    QPixmap flagImage;//红旗
    QPixmap mineImage;//雷

    QPixmap smileImage;//游戏进行中状态图
    QPixmap failImage; //游戏失败状态图
    QPixmap successImage; //成功状态图
    QPixmap rightImage; //正确排除


    bool gameOver; //游戏是否失败，失败显示所有雷
    bool gameSuccess; //游戏成功，对号

    int seconds;
    int mleft;
};
#endif // MAINWINDOW_H
