#ifndef NOEUDC_H
#define NOEUDC_H

#include<vector>
#include<move.h>

#define CLOUAGE_H 1
#define CLOUAGE_V 2
#define CLOUAGE_DIAG 3
#define CLOUAGE_ADIAG 4

// The color is not used by the engine, only for the ZKey and the display.

class NoeudC
{
public:
    NoeudC(Square sq, Piece piece = empty, Color c = white);
    NoeudC(NoeudC const& nodeToCopy);
    static Move* getMoveFromAlgNot(std::string &m, bool white_is_playing, NoeudC* chessB[]);
    Color getColor() const;
    void setColor(Color c);
    Square getSquare() const;
    void setSquare(const Square& sq);
    void setPiece(const Piece& piece);
    BB getPinnedMoves() const;
    void setMove(const BB&);
    BB getMovesBB() const;
    bool hasNonEmptyMove();
    Piece getPiece() const;
    bool isNeverUsed() const;
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
    Color m_color;
    BB m_movesBB;
    BB m_pinnedMovesBB;
    bool m_neverUsed = true;

};

#endif // NOEUDC_H
