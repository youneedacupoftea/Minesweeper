#include "grid.h"

Grid::Grid()
{
  //初始化
    m_pos = QPoint(-1,-1);
    number = 0;
    isMine = false;
    isMarked = false;
    isOpen = false;
}
Grid::Grid(QPoint pos)
{
  //初始化
    m_pos = pos;
    number = 0;
    isMine = false;
    isMarked = false;
    isOpen = false;
}
