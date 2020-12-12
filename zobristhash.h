#ifndef ZOBRISTHASH_H
#define ZOBRISTHASH_H


#include <random>
#include<iostream>
#include "bitboard.h"

class ZobristHash
{
public:
    static void initZobristHash(); // fill every hash array
    static ZKey getPieceHash(Color c, Piece p, Square sq);
    static ZKey getColorHash(const Color& c = black);
    static ZKey getCastlesHash(Color c, bool rightCastle, bool leftCastle);
    static ZKey getEPFileSqHash(Square sq);

    static ZKey PIECE_HASH[2][7][64]; // one hash for each piece at each square (empty square is a pseudo-piece)
    static ZKey CHANGING_SIDE;      // one hash to indicate the side to move
    static ZKey CASTLING_RIGHTS[16];   // [KQkq] (cf FEN notation)
    static ZKey EP_SQ_FILE[8];        // file of the ep square.

    static const CRights RIGHT_CASTLE[2];
    static const CRights LEFT_CASTLE[2];
    // (1) = right castle for white
    // (2) = left castle for white
    // (3) = right castle for black
    // (4) = left castle for black

};

#endif // ZOBRISTHASH_H
