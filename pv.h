#ifndef PV_H
#define PV_H

#include<iostream>
#include "move.h"
#include "iaerror.h"

// For this class, 2 'depth max':
// - DEPTH_MAX is the "real" maximum depth of this principal variation (in allocation term)
// - m_depthMax is the temporary maximum depth (can change often without allocating memory,
//   except if m_depthMax > DEPTH_MAX, which implies a reallocation of memory.

// A right use is to construct the PV with the maximum depth that you expect. You allocate the memory
// only one time.


class PV
{
public:
    PV(int depthMax = 0);
    Move getBestMove();
    std::vector<Move> getPrincipalVariation(int &length);
    void clean(bool deleteObj = true);
    void setDepth(int depth);
    void copyLine(int from, int to, int toShift);
    void setMoveToPly(int ply, Move* m);
    std::string toString(bool allInfos = true);
    ~PV();
private:
    int DEPTH_MAX;
    int m_depthMax;
    Move*** m_pvArray;
};


#endif // PV_H
