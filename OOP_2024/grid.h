#ifndef GRID_H
#define GRID_H

#include<QPoint>

class Grid
{
public:
    Grid();
    Grid(QPoint pos);
    //物体所在位置
    QPoint m_pos;

    //是否是雷
    bool isMine;
    //是否已标记为雷
    bool isMarked;
    //是否已打开，且非雷
    bool isOpen;

    //数字，周围的雷数
    int number;
};

#endif // GRID_H
