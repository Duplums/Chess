#ifndef NOEUDC_H
#define NOEUDC_H

#include<vector>
#include<move.h>

#define CLOUAGE_H 1
#define CLOUAGE_V 2
#define CLOUAGE_DIAG 3
#define CLOUAGE_ADIAG 4

extern const BB one;

class NoeudC
{
public:
    NoeudC(Square sq, Piece piece = empty);
    static Move* getMoveFromAlgNot(std::string &m, bool white_is_playing, NoeudC* chessB[]);
    Square getSquare();
    void setSquare(const Square& sq);
    void setPiece(const Piece& piece);
    void setMove(const BB&);
    BB* getMovesBB();
    bool hasNonEmptyMove();
    Piece getPiece();
    bool isNeverUsed();
    void setNeverUsed(bool b);
    void addMove(Square);
    void setPinnedMove(const BB&);
    void addMove(const BB);
    void andMove(BB &);
    void cleanMovesBB();
    void cleanPinnedMovesBB();
    void toString();

private:
    Square m_sq;
    Piece m_piece;

    BB m_movesBB;
    BB m_pinnedMovesBB;
    bool m_neverUsed = true;

};

#endif // NOEUDC_H
