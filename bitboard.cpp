#include "bitboard.h"


/* Initialisation des variables globales */

const BB one = 1;
const BB indicatrice[2] = {0, ~one ^ one};
const BB petitRoque[2] = {one << 5 | one << 6, one << 61 | one << 62};
const BB grandRoque[2] = {one << 1 | one << 2 | one << 3, one << 57 | one << 58 | one << 59};
const Square rightRookByColor[2] = {h8, h1};
const Square leftRookByColor[2] = {a8, a1};
const BB pawnPromotionRaw[2] = {0xff00000000000000, 0xff};
BB rayAttacks[8][64];
BB pawnAttacks[2][64];
BB pawnMove[2][64];
BB pawnPartialMove[2][64];
BB knightAttacks[64];
BB kingAttacks[64];
BB rookAttacks[64];
BB bishopAttacks[64];
BB diagAttacks[64];
BB antiDiagAttacks[64];
BB HAttacks[64];
BB VAttacks[64];
BB HAdjacent[64];
unsigned char byteOccupancy[256][8];
unsigned char byteFillingOccupancy[256][8];
BB fillCopyOfLastRow[256];
BB fillCopyOfLastFile[256];
BB inBetweenArray[64][64];

const char lookUpLeastSignBit[68] = {
     64,  0,  1, 39,  2, 15, 40, 23,
      3, 12, 16, 59, 41, 19, 24, 54,
      4, -1, 13, 10, 17, 62, 60, 28,
     42, 30, 20, 51, 25, 44, 55, 47,
      5, 32, -1, 38, 14, 22, 11, 58,
     18, 53, 63,  9, 61, 27, 29, 50,
     43, 46, 31, 37, 21, 57, 52,  8,
     26, 49, 45, 36, 56,  7, 48, 35,
      6, 34, 33, -1};

Square BitBoard::getIndexLeastSignBit(const BB &bb){
    return (Square)lookUpLeastSignBit[(bb & -bb) % 67];
}

void BitBoard::flipVertical(){
    const int64_t k1 = 0x00ff00ff00ff00ff;
    const int64_t k2 = 0x0000ffff0000ffff;
    m_bb = ((m_bb >>  8) & k1) | ((m_bb & k1) <<  8);
    m_bb = ((m_bb >> 16) & k2) | ((m_bb & k2) << 16);
    m_bb = ( m_bb >> 32)       | ( m_bb       << 32);
}

void BitBoard::flipHorizontal(){
    const int64_t k1 = 0x5555555555555555;
    const int64_t k2 = 0x3333333333333333;
    const int64_t k4 = 0x0f0f0f0f0f0f0f0f;
    m_bb = ((m_bb >> 1) & k1) | ((m_bb & k1) << 1);
    m_bb = ((m_bb >> 2) & k2) | ((m_bb & k2) << 2);
    m_bb = ((m_bb >> 4) & k4) | ((m_bb & k4) << 4);
}

void BitBoard::flipDiag(){
    int64_t k1 = 0x5500550055005500;
    int64_t k2 = 0x3333000033330000;
    int64_t k4 = 0x0f0f0f0f00000000;
    int64_t t;
    t  = k4 & (m_bb ^ (m_bb << 28));
    m_bb ^=       t ^ (t >> 28) ;
    t  = k2 & (m_bb ^ (m_bb << 14));
    m_bb ^=       t ^ (t >> 14) ;
    t  = k1 & (m_bb ^ (m_bb <<  7));
    m_bb ^=       t ^ (t >>  7) ;
}

void BitBoard::flipAntiDiag(){
    const int64_t k1 = 0xaa00aa00aa00aa00;
    const int64_t k2 = 0xcccc0000cccc0000;
    const int64_t k4 = 0xf0f0f0f00f0f0f0f;
    int64_t t;
    t  =       m_bb ^ (m_bb << 36) ;
    m_bb ^= k4 & (t ^ (m_bb >> 36));
    t  = k2 & (m_bb ^ (m_bb << 18));
    m_bb ^=       t ^ (t >> 18) ;
    t  = k1 & (m_bb ^ (m_bb <<  9));
    m_bb ^=       t ^ (t >>  9) ;
}

void BitBoard::initPawnAttacks(){
    for(char i = 0; i < 8; ++i){
        for(char j = 0; j < 8; ++j){
            pawnAttacks[white][8*i+j] = 0;
            pawnAttacks[black][8*i+j] = 0;
            // white pawn attack
            if(i > 0 && j%8 < 7)
                pawnAttacks[white][8*i+j] = BB(one << (8*(i-1)+j+1));
            if(i > 0 && j%8 > 0)
                pawnAttacks[white][8*i+j] |= BB(one << (8*(i-1)+j-1));
            // black pawn attack
            if(i < 7 && j%8 < 7)
                pawnAttacks[black][8*i+j] = BB(one << (8*(i+1)+j+1));
            if(i < 7 && j%8 > 0)
                pawnAttacks[black][8*i+j] |= BB(one << (8*(i+1)+j-1));
        }
    }
}

void BitBoard::initPawnMove(){
    for(char sq = 0; sq < 64; ++sq){
        pawnMove[white][sq] = 0;
        pawnMove[black][sq] = 0;
        if(sq/8 == 1){
            pawnMove[black][sq] = one << (sq+8) | one << (sq+16);
            pawnMove[white][sq] = one << (sq-8);
        }
        else if(sq/8 == 6){
            pawnMove[white][sq] = one << (sq-8) | one << (sq-16);
            pawnMove[black][sq] = one << (sq+8);
        }
        else if(sq/8 == 0)
            pawnMove[black][sq] = one << (sq+8);
        else if(sq/8 == 7)
            pawnMove[white][sq] = one << (sq-8);
        else{
            pawnMove[white][sq] = one << (sq-8);
            pawnMove[black][sq] = one << (sq+8);
        }
    }
}

void BitBoard::initPawnPartialMove(){
    for(char sq = 0; sq < 64; ++sq){
        pawnPartialMove[white][sq] = 0;
        pawnPartialMove[black][sq] = 0;
        if(sq/8 == 0)
            pawnPartialMove[black][sq] = one << (sq+8);
        else if(sq/8 == 7)
            pawnPartialMove[white][sq] = one << (sq-8);
        else{
            pawnPartialMove[white][sq] = one << (sq-8);
            pawnPartialMove[black][sq] = one << (sq+8);
        }
    }
}

// can be optimized
void BitBoard::initKnightAttacks(){
    const signed char X[4] = {-2,-1,1,2};
    signed char Y[2];
    for(char x = 0; x < 8; ++x){
        for(char y = 0; y < 8; ++y){
            knightAttacks[8*x+y] = 0;
            for(signed char epsX : X){
                Y[0] = 3 - std::abs(epsX); Y[1] =  -(3 - std::abs(epsX));
                for(signed char epsY : Y){
                    if(x+epsX >= 0 && y+epsY >= 0 && x+epsX < 8 && y+epsY < 8)
                        knightAttacks[8*x+y] |= (one << (8*(x+epsX)+y+epsY));
                }
            }
        }
    }
}

void BitBoard::initRookAttacks(){
    for(char sq = 0; sq < 64; ++sq){
        rookAttacks[sq] = rayAttacks[N][sq] | rayAttacks[S][sq] | rayAttacks[W][sq] | rayAttacks[E][sq];
    }
}

void BitBoard::initBishopAttacks(){
    for(char sq = 0; sq < 64; ++sq){
        bishopAttacks[sq] = rayAttacks[SE][sq] | rayAttacks[SW][sq] | rayAttacks[NE][sq] | rayAttacks[NW][sq];
    }
}

void BitBoard::initByteOccupancy(){
    unsigned char one = 1;
    unsigned char rightShiftMask = one;
    unsigned char leftShiftMask = one;
    for(unsigned int byte = 0; byte < 256; ++byte){
        for(unsigned  char sq = 0; sq < 8; ++sq){
            // right part byte [sq ...]
            if(sq < 7){
                rightShiftMask = one << (sq + 1);
                for(unsigned char rightShift = sq+1; rightShift < 8 && !(rightShiftMask & byte); ++rightShift, rightShiftMask <<= 1){}
                byteOccupancy[byte & 255][sq] |= rightShiftMask & byte;
            }
            if(sq > 0){
                // left part byte [... sq]
                leftShiftMask = one << (sq - 1);
                for(signed char leftShift = sq-1; leftShift >= 0 && !(leftShiftMask & byte); --leftShift, leftShiftMask >>= 1){}
                byteOccupancy[byte & 255][sq] |= leftShiftMask & byte;
            }
        }
    }
}

void BitBoard::initByteFillingOccupancy(){
    unsigned char one = 1;
    unsigned char rightShiftMask = one;
    unsigned char leftShiftMask = one;
    for(unsigned int byte = 0; byte < 256; ++byte){
        for(unsigned  char sq = 0; sq < 8; ++sq){
            // right part byte [sq ...]
            if(sq < 7){
                rightShiftMask = one << (sq + 1);
                for(unsigned char rightShift = sq+1; rightShift < 8 && !(rightShiftMask & byte); ++rightShift, rightShiftMask <<= 1){
                    byteFillingOccupancy[byte & 255][sq] |= rightShiftMask;
                }
                byteFillingOccupancy[byte & 255][sq] |= rightShiftMask & byte;
            }
            if(sq > 0){
                // left part byte [... sq]
                leftShiftMask = one << (sq - 1);
                for(signed char leftShift = sq-1; leftShift >= 0 && !(leftShiftMask & byte); --leftShift, leftShiftMask >>= 1){
                    byteFillingOccupancy[byte & 255][sq] |= leftShiftMask;
                }
                byteFillingOccupancy[byte & 255][sq] |= leftShiftMask & byte;
            }
        }
    }
}

void BitBoard::initRayAttacks(){
    BB mask(0xff); // first row

    // north attack
    BB attack(0x0080808080808080); // last column fill, excepted h1
    for(char i = 63; i >= 0; --i, attack >>= 1){
        rayAttacks[N][i] = attack;
    }
    // south attack
    attack = 0x0101010101010100; // first column fill, excepted a8
    for(char i = 0; i < 64; ++i, attack <<= 1){
        rayAttacks[S][i] = attack;
    }

    // east attack
    attack = 0xfe; // first row 0111 1111
    for(char i = 0; i < 8; ++i, attack = mask & 0xfe << i){
        for(char j = 0; j < 8; ++j, attack <<= 8){
            rayAttacks[E][8*j+i] = attack;
        }
    }

    // west attack
    attack = 0x7f; // first row 1111 1110
    for(char i = 0; i < 8; ++i, attack = mask & 0x7f >> i){
        for(char j = 0; j < 8; ++j, attack <<= 8){
            rayAttacks[W][8*j+7-i] = attack;
        }
    }

    // south east attack
    attack = 0x8040201008040200; // diag filled, excepted a8
    mask =   0xffffffffffffffff;
    for(char i = 0 ; i < 8; ++i, mask >>= 8, attack = (0x8040201008040200 & mask) << i){
        for(char j = 0; j < 8; ++j, attack <<= 8){
           rayAttacks[SE][8*j+i] = attack;
        }
    }

    // south west attack
    attack = 0x0102040810204000; // anti-diag filled, excepted h8
    mask =   0xffffffffffffffff;
    for(char i = 0 ; i < 8; ++i, mask >>= 8, attack = (0x0102040810204000 & mask) >> i){
        for(char j = 0; j < 8; ++j, attack <<= 8){
            rayAttacks[SW][8*j+7-i] = attack;
        }
    }

    //north east attack
    attack = 0x02040810204080; // anti-diag filled, excepted a1
    mask = 0xffffffffffffff;
    for(char i = 0 ; i < 8; ++i, mask >>= 8, attack = (0x02040810204080 & mask) << 8*i){
        for(char j = 7; j >= 0; --j, attack >>= 8){
            rayAttacks[NE][8*j+i] = attack;
        }
    }

    // north west attack
    attack = 0x40201008040201; // diag filled, excepted h1
    mask = 0xffffffffffffffff;
    for(char i = 0 ; i < 8; ++i, mask <<= 8, attack = (0x40201008040201 & mask) >> i){
        for(char j = 7; j >= 0; --j, attack >>= 8){
            rayAttacks[NW][8*j+7-i] = attack;
        }
    }
}

// We assume rayAttacks is initialized
// space saving : use the symetry of this array.
// We keep the "to" piece in the "inBetweenArray"
void BitBoard::initInBetweenArray(){
    char iFrom, jFrom, iTo, jTo;
    for(char from = a8; from <= h1; ++from){
        for(char to = a8 ; to <= h1; ++to){
            inBetweenArray[from][to] = BB(0);
            iFrom = from/8; jFrom = from%8;
            iTo = to/8; jTo = to%8;
            char distance = std::max(std::abs(iFrom-iTo), std::abs(jFrom-jTo));
            if(distance > 0){
                // align diag
                if(iFrom-iTo == jFrom-jTo)
                    inBetweenArray[from][to] = (rayAttacks[SE][from] & rayAttacks[NW][to] | rayAttacks[NW][from] & rayAttacks[SE][to])
                                                ^ (one << to);
                // align anti diag
                else if(iFrom-iTo == jTo-jFrom)
                    inBetweenArray[from][to] = (rayAttacks[SW][from] & rayAttacks[NE][to] | rayAttacks[NE][from] & rayAttacks[SW][to])
                                                ^ (one << to);
               // align horizontal
                else if(iFrom == iTo)
                    inBetweenArray[from][to] = (rayAttacks[E][from] & rayAttacks[W][to] | rayAttacks[W][from] & rayAttacks[E][to])
                                                ^ (one << to);
                // align vertical
                else if(jFrom == jTo)
                    inBetweenArray[from][to] = (rayAttacks[S][from] & rayAttacks[N][to] | rayAttacks[N][from] & rayAttacks[S][to])
                                                ^ (one << to);
            }
        }
    }
}


void BitBoard::initKingAttacks(){
    BB notLeftFile = 0xfefefefefefefefe;
    BB notRightFile = 0x7f7f7f7f7f7f7f7f;
    BB oneShift(one);
    for(char sq = 0; sq < 64; ++sq, oneShift <<= 1){
        kingAttacks[sq] = (oneShift << 1) &  notLeftFile | (oneShift >> 1) & notRightFile; // horizontal
        kingAttacks[sq] |= (oneShift << 8) | (oneShift >> 8); // vertical
        kingAttacks[sq] |= ((oneShift << 7) | (oneShift >> 9)) & notRightFile; // diag, anti-diag
        kingAttacks[sq] |= ((oneShift << 9) | (oneShift >> 7)) & notLeftFile;// diag, anti-diag
    }
}

void BitBoard::initDiagAttacks(){
    for(char sq = 0; sq < 64; ++sq){
        diagAttacks[sq] = rayAttacks[NW][sq] | rayAttacks[SE][sq];
    }
}

void BitBoard::initAntiDiagAttacks(){
    for(char sq = 0; sq < 64; ++sq){
        antiDiagAttacks[sq] = rayAttacks[NE][sq] | rayAttacks[SW][sq];
    }
}

void BitBoard::initHAttacks(){
    for(char sq = 0; sq < 64; ++sq){
        HAttacks[sq] = rayAttacks[E][sq] | rayAttacks[W][sq];
    }
}

void BitBoard::initHAdjacent(){
    for(char sq = 0; sq < 64; ++sq){
        HAdjacent[sq] = HAttacks[sq] & kingAttacks[sq];
    }
}

void BitBoard::initVAttacks(){
    for(char sq = 0; sq < 64; ++sq){
        VAttacks[sq] = rayAttacks[N][sq] | rayAttacks[S][sq];
    }
}

void BitBoard::init(){
    initRayAttacks();
    initPawnAttacks();
    initPawnMove();
    initPawnPartialMove();
    initKnightAttacks();
    initRookAttacks();
    initBishopAttacks();
    initKingAttacks();
    initDiagAttacks();
    initAntiDiagAttacks();
    initHAttacks();
    initHAdjacent();
    initVAttacks();
    initByteOccupancy();
    initByteFillingOccupancy();
    initInBetweenArray();
    initFillCopyOfLastRow();
    initFillCopyOfLastFile();
}


void BitBoard::initFillCopyOfLastRow(){
    for(BB byte = 0; byte < 256; ++byte){
        fillCopyOfLastRow[byte & 255] = BB(byte | byte << 8 | byte << 16 | byte << 24 | byte << 32 | byte << 40 | byte << 48 | byte << 56);
    }
}

// From : byte <-> 1 0 0 1 1 0 0 0
// To :
//  0 0 0 0 0 0 0 0
//  0 0 0 0 0 0 0 0
//  0 0 0 0 0 0 0 0
//  1 1 1 1 1 1 1 1
//  1 1 1 1 1 1 1 1
//  0 0 0 0 0 0 0 0
//  0 0 0 0 0 0 0 0
//  1 1 1 1 1 1 1 1
void BitBoard::initFillCopyOfLastFile(){
    BB mask(0x8040201008040201);
    BB firstFile(0x0101010101010101);
    BB result;
    for(BB byte = 0; byte < 256; ++byte){
        result = ((byte * mask) >> 7) & firstFile;
        fillCopyOfLastFile[byte & 255] = result | result << 1 | result << 2 | result << 3 | result << 4 | result << 5 | result << 6 | result << 7;
    }
}

// On suppose que bb est déjà sous forme diagonale
void BitBoard::diagClosestBitsFromSq(BB& diagMask, BB& bb, Square sq){
    const BB mask(0x0101010101010101);
    bb = (bb * mask) >> 56;
    bb = fillCopyOfLastRow[byteOccupancy[bb & 255][sq & 7]] & diagMask;
}
// Idem sous forme anti-diag
void BitBoard::antiDiagClosestBitsFromSq(BB& antiDiagMask, BB& bb, Square sq){
    diagClosestBitsFromSq(antiDiagMask, bb, sq);
}
// Idem
void BitBoard::rankClosestBitsFromSq(BB& bb, Square sq){
    unsigned char shift = 8*(sq/8);
    bb = byteOccupancy[(bb >> shift) & 255][sq & 7];
    bb <<= shift;
}
// Idem
void BitBoard::fileClosestBitsFromSq(BB& fileMask, BB& bb, Square sq){
   const BB mask(0x8040201008040201);
   bb = ((bb >> (sq & 7)) * mask) >> 56;
   bb = fillCopyOfLastFile[byteOccupancy[bb & 255][7-sq/8]] & fileMask;
}

std::string BitBoard::toString(const Square sq){
    // make the translation from square to string
    uchar letter = sq%8 + 97;
    uchar nb = 8 - sq/8 + 48;
    char letterC = static_cast<char>(letter);
    char nbC = static_cast<char>(nb);
    std::string resultString(&letterC, 1);
    resultString.append(&nbC, 1);
    return resultString;
}

std::string BitBoard::toString(const BB bb){
    BB mask(1);
    std::string result = "\n ######################";
    for(int i = 0; i < 64 ; ++i){
        if(i%8 == 0)
            result += "\n";
        if(mask & bb ){
            result += " 1 ";
        }
        else
            result += " . ";
        mask <<= 1;
    }
    std::cout << result + " \n ######################\n";
    return result;
}


