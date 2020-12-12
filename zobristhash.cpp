#include "zobristhash.h"

ZKey ZobristHash::PIECE_HASH[2][7][64]; // one hash for each piece at each square
ZKey ZobristHash::CHANGING_SIDE;      // one hash to indicate the side to move
ZKey ZobristHash::CASTLING_RIGHTS[16];   // [KQkq] (cf FEN notation)
ZKey ZobristHash::EP_SQ_FILE[8];        // file of the ep square.
const CRights ZobristHash::RIGHT_CASTLE[2] = {4, 1};
const CRights ZobristHash::LEFT_CASTLE[2] = {8, 2};

void ZobristHash::initZobristHash(){
    std::default_random_engine m_generator;
    std::uniform_int_distribution<uint64_t> m_unifDistrib(0, std::numeric_limits<uint64_t>::max());

    for(int color = 0; color < 2; ++color){
        for(int pieceKind = 0; pieceKind < 7; ++pieceKind){
            for(int pieceSq = 0; pieceSq < 64; ++pieceSq){
                PIECE_HASH[color][pieceKind][pieceSq] = m_unifDistrib(m_generator);
            }
        }
    }
    CHANGING_SIDE = m_unifDistrib(m_generator);
    for(int k = 0; k < 16; ++k)
        CASTLING_RIGHTS[k] = m_unifDistrib(m_generator);
    for(int file = 0; file < 8; ++file)
        EP_SQ_FILE[file] = m_unifDistrib(m_generator);
}

ZKey ZobristHash::getCastlesHash(Color c, bool rightCastle, bool leftCastle){
    if(!rightCastle && !leftCastle)
        return 0;
    if(rightCastle && leftCastle){
        std::cout << "Warning: getCastlesHash for both right and left castle, returned 0" << std::endl;
        return 0;
    }
    return CASTLING_RIGHTS[(rightCastle ? RIGHT_CASTLE[c] : LEFT_CASTLE[c])&15];
}

ZKey ZobristHash::getColorHash(const Color &c){
    if(c == white)
        return 0;
    return CHANGING_SIDE;
}

ZKey ZobristHash::getEPFileSqHash(Square sq){
    if(sq < outside && sq >= 0)
        return EP_SQ_FILE[sq%8];
    return 0;
}

ZKey ZobristHash::getPieceHash(Color c, Piece p, Square sq){
    if(p != empty && sq < outside && sq >= 0)
        return PIECE_HASH[c][p][sq];
    return 0;
}
