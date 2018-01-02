#ifndef BITBOARD_H
#define BITBOARD_H

#include<iostream>
#include<string>
#include<QDebug>

typedef enum Color : bool{
    black = false,
    white = true
} Color;

typedef enum PieceShift : char{
    pawnShift = 0,
    rookShift = 1,
    knightShift = 2,
    bishopShift = 3,
    queenShift = 4,
    kingShift = 5
} PieceShift;

typedef enum Direction: char{
    N = 0, E = 1, W = 2, S = 3,
    NE = 4, NW = 5, SE = 6, SW = 7, NONE = 8
} Direction ;

typedef enum Square: unsigned char{
     a1 = 56,  a2 = 48,  a3 = 40,  a4 = 32,  a5 = 24,  a6 = 16,  a7 = 8,  a8 = 0,
     b1 = 57,  b2 = 49,  b3 = 41,  b4 = 33,  b5 = 25,  b6 = 17,  b7 = 9,  b8 = 1,
     c1 = 58,  c2 = 50,  c3 = 42,  c4 = 34,  c5 = 26,  c6 = 18,  c7 = 10,  c8 = 2,
     d1 = 59,  d2 = 51,  d3 = 43,  d4 = 35,  d5 = 27,  d6 = 19,  d7 = 11,  d8 = 3,
     e1 = 60,  e2 = 52,  e3 = 44,  e4 = 36,  e5 = 28,  e6 = 20,  e7 = 12,  e8 = 4,
     f1 = 61,  f2 = 53,  f3 = 45,  f4 = 37,  f5 = 29,  f6 = 21,  f7 = 13,  f8 = 5,
     g1 = 62,  g2 = 54,  g3 = 46,  g4 = 38,  g5 = 30,  g6 = 22,  g7 = 14,  g8 = 6,
     h1 = 63,  h2 = 55,  h3 = 47,  h4 = 39,  h5 = 31,  h6 = 23,  h7 = 15,  h8 = 7,
     outside = 64
}Square;

// A voir...
typedef enum Piece : char{
    empty = 0,
    pawn = 1,  rook = 2, knight = 3, bishop = 4, queen = 5, king = 6,
} Piece;

typedef uint64_t BB;
typedef uint32_t Flag;
typedef uint32_t MoveE;


/* Déclaration des variables globales */

/* Total : 60 ko */
extern const BB one;

extern BB rayAttacks[8][64];  // 4ko
extern BB pawnAttacks[2][64]; // 1ko
extern BB pawnMove[2][64];    // 1ko;     paw move 1 step forward except for the initial positionning pawns: 2 steps forward
extern BB pawnPartialMove[2][64]; // 1ko, pawn move 1 step forward
extern BB knightAttacks[64];  // 0.5ko
extern BB kingAttacks[64];    // 0.5ko
extern BB rookAttacks[64];    // 0.5ko
extern BB bishopAttacks[64];  // 0.5ko
/*(INITIALIZED AFTER RAYATTACKS)*/
extern BB diagAttacks[64];    // 0.5ko
extern BB antiDiagAttacks[64];// 0.5ko
extern BB HAttacks[64];       // 0.5ko
extern BB VAttacks[64];       // 0.5ko
extern BB HAdjacent[64];      // 0.5ko (INITIALIZED AFTER HATTACKS AND KINGATTACKS)
// For a line and a sq on that line, returns the closest bits of the sq
// (can be optimized in memory term)
extern uchar byteOccupancy[256][8]; //2ko
extern uchar byteFillingOccupancy[256][8]; //2ko
extern BB fillCopyOfLastRow[256];  //2ko
extern BB fillCopyOfLastFile[256]; //2ko
extern BB inBetweenArray[64][64]; //32 ko... (INITIALIZED AFTER RAYATTACKS)

extern const BB indicatrice[2];
// Castling
extern const BB petitRoque[2];
extern const BB grandRoque[2];
extern const Square rightRookByColor[2];
extern const Square leftRookByColor[2];

// Promotion
extern const BB pawnPromotionRaw[2];
extern const char lookUpLeastSignBit[68];

class BitBoard
{
public:
    static Square getIndexLeastSignBit(const BB &bb);
    void flipVertical();
    void flipHorizontal();
    void flipDiag();
    void flipAntiDiag();

    void initRayAttacks();
    void initPawnAttacks();
    void initPawnMove();
    void initPawnPartialMove();
    void initKnightAttacks();
    void initBishopAttacks();
    void initRookAttacks();
    void initDiagAttacks();
    void initAntiDiagAttacks();
    void initVAttacks();
    void initHAttacks();
    void initHAdjacent();
    void initKingAttacks();
    void initByteOccupancy();
    void initByteFillingOccupancy();
    void initInBetweenArray();
    void initFillCopyOfLastRow();
    void initFillCopyOfLastFile();

    void init();

    /* Etant donné sq et un bb, donne les bits les plus proches
     * de notre sq selon le type de ligne*/

    static void diagClosestBitsFromSq(BB &diagMask, BB& bb,Square sq);
    static void antiDiagClosestBitsFromSq(BB &antiDiagMask, BB& bb, Square sq);
    static void rankClosestBitsFromSq(BB& bb, Square sq);
    static void fileClosestBitsFromSq(BB &fileMask, BB& bb, Square sq);

    static std::string toString(const BB bb);
    static std::string toString(const Square sq);
private:
    BB m_bb;
};




#endif // BITBOARD_H
