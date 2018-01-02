#ifndef PV_H
#define PV_H

#include<iostream>
#include "move.h"
#include "iaerror.h"

class PV
{
public:
    PV(int depthMax = 0);
    void clean();
    void setDepth(int depth);
    void copyLine(int from, int to, int toShift) throw(IAError);
    void setMoveToPly(int ply, Move* m) throw(IAError);
    ~PV();
private:
    int m_depthMax;
    Move*** m_pvArray;
};


#endif // PV_H
