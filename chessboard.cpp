#include "chessboard.h"

const char ChessBoard::doublePushPawn[2] = {16, -16};
const char ChessBoard::doublePushEPSq[2] = {-8, 8};

ChessBoard::ChessBoard(){
    /*Initialisation des bitboards et des clés de hash */
    ZobristHash::initZobristHash();
    BitBoard::init();
    clean();
    m_movesGen = new MovesGenerator();
}

// On fait un deep copy de chessboard (réallocations mémoires...)

ChessBoard::ChessBoard(const ChessBoard &boardToCopy){
    // On commence par les choses simples.
    this->m_chessBoard = boardToCopy.getChessBB();
    this->m_whiteIsPlaying = boardToCopy.getColorToPlay();
    this->m_plySearch = boardToCopy.getPlySearch();
    this->m_depthMax = boardToCopy.getMaxDepth();
    this->m_zKey = boardToCopy.getZobristKey();
    this->m_castlingRights = boardToCopy.getCastlingRights();
    this->m_movesGen = new MovesGenerator(*boardToCopy.getMovesGen());
    for(uchar c = black; c < 2; ++c){
        this->m_indexDeadPieces[c] = boardToCopy.getIndexDeadPieces((Color)c);
        this->m_chessBB_[c] = boardToCopy.getChessBBPerColor((Color)c);
        this->m_doublePushSq[c] = boardToCopy.getDoublePushSq((Color)c);

        for(uchar kind = pawnShift; kind <= kingShift; ++kind)
            this->m_chessBB[c][kind] = boardToCopy.getChessBBPerColorAndKind((Color)c, (PieceShift)kind);

        // Réallocation mémoire des pièces pour assurer une étanchéité des threads.
        for(uchar k = 0; k < 16; ++k){
            if(boardToCopy.getDeadPiece((Color)c, k) != NULL){
                this->m_deadPiecesBB[c][k] = new NoeudC(*boardToCopy.getDeadPiece((Color)c, k));
            }else
                this->m_deadPiecesBB[c][k] = NULL;
        }
    }
    for(uchar sq = 0; sq < 64; ++sq){
        if(boardToCopy.getChessBPiece((Square)sq) != NULL)
            m_chessB[sq] = new NoeudC(*boardToCopy.getChessBPiece((Square)sq));
        else
            m_chessB[sq] = NULL;
    }
}

// return tree if succeed to init the chessboard
bool ChessBoard::initChessBoard(){
    bool succeed = setPosition(std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    if(!succeed)
        clean();
    return succeed;
}

// set the chessboard into a situation.
// The engine passed all these tests for a min depth of 6.
bool ChessBoard::testPosition(int k){
    switch(k){
        case 1:    // cf chessprogramming perft section - position 2 "Kiwipete" [CASTLES]
            return setPosition(std::string("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"));
        case 2:    // cf chessprogramming perft section - position 3 [CHECK AND CAPTURES]
            return setPosition(std::string("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"));
        case 3:    // cf chessprogramming perft section - position 4 [PROMOTIONS]
            return setPosition(std::string("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"));
        case 4:    // cf chessprogramming perft section - position 5
            return setPosition(std::string("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"));
        case 5:    // cf chessprogramming perft section - position 6
            return setPosition(std::string("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"));
        default:
            return false;
    }
}


// Set a position using FEN notation, returns false if there're errors.
// It inits the zKey.
bool ChessBoard::setPosition(const std::string &pos, bool cleanInitAndDeletePointers){
    std::regex notation("(([kqrbnpKQRBNP1-8]+)\/){7}([kqrbnpKQRBNP1-8]+) [wb] (K?Q?k?q?|-) ([a-h][3 6]|-)( (50|[1-4]?[0-9]))?( (100|[1-9]?[0-9]))?");
    if(std::regex_match(pos, notation)){
        if(cleanInitAndDeletePointers){
            clean(true);
            m_movesGen = new MovesGenerator();
        }
        int pieceNb[2][6] = {{0,0,0,0,0,0},{0,0,0,0,0,0}}; // Nb of pieces set per kind and color.
        int maxPieceNb[6] = {8, 2+8, 2+8, 2+8, 1+8, 1}; //(+8) in case of promotion...
        std::regex nb("[1-8]"), blackPiece("[kqrbnp]"), whitePiece("[KQRBNP]"), endOfLine("[ \/]");
        std::map<std::string, PieceShift> pieceMapping;
        pieceMapping.insert(std::make_pair("p", pawnShift));
        pieceMapping.insert(std::make_pair("r", rookShift));
        pieceMapping.insert(std::make_pair("b", bishopShift));
        pieceMapping.insert(std::make_pair("q", queenShift));
        pieceMapping.insert(std::make_pair("k", kingShift));
        pieceMapping.insert(std::make_pair("n", knightShift));
        std::string piece;
        uint globalCursor = 0, localCursor = 0, txtCursor = 0;
        NoeudC* leftRook[2]={NULL, NULL};
        NoeudC* rightRook[2]={NULL, NULL};
        NoeudC* king[2]={NULL, NULL};
        for(int k = 0; k < 8;){ // set pieces rank by rank.
            piece = pos.substr(txtCursor++, 1);
            if(std::regex_match(piece, nb)){
                // set nb of empty squares
                if(localCursor + std::stoi(piece) > 8){
                    std::cout << "Wrong nb of empty square at line " << k << std::endl;
                    clean();
                    return false;
                }
                localCursor += std::stoi(piece);
                globalCursor += std::stoi(piece);
            } else if(std::regex_match(piece, blackPiece) || std::regex_match(piece, whitePiece)){
                if(++localCursor > 8){
                    std::cout << "Wrong nb of pieces at line "<< k << std::endl;
                    clean();
                    return false;
                }
                Color c = (std::regex_match(piece, blackPiece) ? black : white) ;
                std::transform(piece.begin(), piece.end(), piece.begin(),
                               [](unsigned char c) -> unsigned char { return std::tolower(c); });
                PieceShift myPiece = pieceMapping[piece];
                if(++pieceNb[c][myPiece] > maxPieceNb[myPiece]){
                    std::cout << "Too many " << piece << (c == black ? " black" : " white") << " pieces" << std::endl;
                    clean();
                    return false;
                }
                m_chessB[globalCursor] = new NoeudC((Square)globalCursor, (Piece)(myPiece+1), c); // PieceShift <-> Piece - 1
                m_chessBB[c][myPiece] |= (one << globalCursor);

                if(myPiece == rookShift){
                    m_chessB[globalCursor]->setNeverUsed(false);
                    if(globalCursor == leftRookByColor[c])
                        leftRook[c] = m_chessB[globalCursor];
                    else if(globalCursor == rightRookByColor[c])
                        rightRook[c] = m_chessB[globalCursor];
                } else if(myPiece == kingShift){
                    m_chessB[globalCursor]->setNeverUsed(false);
                    king[c] = m_chessB[globalCursor];
                }

                globalCursor += 1;
            } else if(std::regex_match(piece, endOfLine)){
                if(localCursor != 8){
                    std::cout << "Wrong nb of square line " << k << std::endl;
                    clean();
                    return false;
                }
                k ++;
                localCursor = 0;
            } else {
                std::cout << "Unknown notation for " << piece << std::endl;
                clean();
                return false;
            }
        }
        // Final check about nb of pieces on the chessboard
        uint totalNbPieces[2] = {0, 0};
        for(int k = 0; k < 6; ++k){
            totalNbPieces[white] += pieceNb[white][k];
            totalNbPieces[black] += pieceNb[black][k];
        }
        if(totalNbPieces[white] > 16 || totalNbPieces[black] > 16){
            std::cout << "Too many pieces on the board (" <<
                         totalNbPieces[white] << " white, " <<
                         totalNbPieces[black] << " black)" << std::endl;
            clean();
            return false;
        } else if(king[white] == NULL || king[black] == NULL){
            std::cout << "Black or white king absent." << std::endl;
        }
        piece = pos.substr(txtCursor++, 1);
        // Now, define external variables

        if(std::regex_match(piece, std::regex("[wb]")))
            m_whiteIsPlaying = (Color)(piece.compare("w") == 0);
        else{
            std::cout << "Wrong player (" << piece << ")" << std::endl;
            clean();
            return false;
        }
        txtCursor++;
        // now split the end of 'pos' with " "

        std::vector<std::string> endOfPos;
        split(pos.substr(txtCursor), std::regex(" "), endOfPos);
        if(endOfPos.size() > 4 || endOfPos.size() < 2){
            std::cout << "Problem at the end of the FEN" << std::endl;
            return false;
        }
        m_castlingRights = 0;
        if(endOfPos[0].compare("-") != 0){
            // castle(s) possible
            if(endOfPos[0].find("K") != std::string::npos){ // right castle for white
                if(rightRook[white] == NULL){
                    std::cout << "No right castle for white" << std::endl;
                    return false;
                }
                rightRook[white]->setNeverUsed(true);
                king[white]->setNeverUsed(true);
                m_castlingRights |= ZobristHash::RIGHT_CASTLE[white];
            }
            if(endOfPos[0].find("Q") != std::string::npos){ // left castle for white
                if(leftRook[white] == NULL){
                    std::cout << "No left castle for white" << std::endl;
                    return false;
                }
                leftRook[white]->setNeverUsed(true);
                king[white]->setNeverUsed(true);
                m_castlingRights |= ZobristHash::LEFT_CASTLE[white];
            }
            if(endOfPos[0].find("k") != std::string::npos){ // right castle for black
                if(rightRook[black] == NULL){
                    std::cout << "No right castle for black" << std::endl;
                    return false;
                }
                rightRook[black]->setNeverUsed(true);
                king[black]->setNeverUsed(true);
                m_castlingRights |= ZobristHash::RIGHT_CASTLE[black];
            }
            if(endOfPos[0].find("q") != std::string::npos){ // left castle for black
                if(leftRook[black] == NULL){
                    std::cout << "No left castle for black" << std::endl;
                    return false;
                }
                leftRook[black]->setNeverUsed(true);
                king[black]->setNeverUsed(true);
                m_castlingRights |= ZobristHash::LEFT_CASTLE[black];
            }
        } else {
            // No possible castle, neither for black or white

        }
        if(endOfPos[1].compare("-") != 0){
            // Possible ep square
            if(std::regex_match(endOfPos[1], std::regex("[a-h][3 6]"))){
                Square sq = (Square)static_cast<unsigned char>((endOfPos[1][0] - 'a') + ('8' - endOfPos[1][1])*8);
                m_doublePushSq[!m_whiteIsPlaying] = sq;
            }else{
                std::cout << "Wrong square notation for ep square (" << endOfPos[1] << ")" << std::endl;
                return false;
            }
        }

        // For the moment, don't check the nb of ply from the beginning of the game
        // (the 50 plys rule is not check here).

        m_chessBB_[white] = m_chessBB[white][pawnShift] | m_chessBB[white][knightShift] | m_chessBB[white][bishopShift] |
                            m_chessBB[white][rookShift] | m_chessBB[white][queenShift]  | m_chessBB[white][kingShift];
        m_chessBB_[black] = m_chessBB[black][pawnShift] | m_chessBB[black][knightShift] | m_chessBB[black][bishopShift] |
                            m_chessBB[black][rookShift] | m_chessBB[black][queenShift]  | m_chessBB[black][kingShift];

        m_chessBoard = m_chessBB_[white] | m_chessBB_[black];
        initZKeyFromScratch();
    }else{
        std::cout << "Wrong notation" << std::endl;
        return false;
    }
    return true;
}

void ChessBoard::split(const std::string &s, const std::regex &r, std::vector<std::string> &result){
    std::string group("");
    result.clear();
    for(int k = 0; k < s.length(); ++k){
        if(std::regex_match(s.substr(k, 1), r)){
            if(group.length() > 0){
                result.push_back(group);
                group.clear();
            }
        }else
            group.append(s.substr(k, 1));
    }
    if(group.length() > 0)
        result.push_back(group);
}

void ChessBoard::clean(bool deletePointers){
    m_whiteIsPlaying = white;
    m_chessBB_[white] = 0;
    m_chessBB_[black] = 0;
    m_castlingRights = 0;
    m_chessBoard = 0;
    for(char sq = 0; sq < 64; ++sq){
        if(deletePointers)
            delete m_chessB[sq];
        m_chessB[sq] = NULL;
    }
    for(char piece = 0; piece <= kingShift; ++piece){
        m_chessBB[white][piece] = 0;
        m_chessBB[black][piece] = 0;
    }
    for(char sq = 0; sq < 16; ++sq){
        if(deletePointers){
            delete m_deadPiecesBB[black][sq];
            delete m_deadPiecesBB[white][sq];
        }
        m_deadPiecesBB[black][sq] = NULL;
        m_deadPiecesBB[white][sq] = NULL;
    }
    m_indexDeadPieces[white] = 0;
    m_indexDeadPieces[black] = 0;
    m_doublePushSq[white] = outside;
    m_doublePushSq[black] = outside;
    m_plySearch = 0;
    m_zKey = 0;
    if(deletePointers)
        delete m_movesGen;
    m_movesGen = NULL;
}

void ChessBoard::setPlySearch(int ply){
    m_plySearch = ply;
}
void ChessBoard::setDepthMax(int depth){
    m_depthMax = depth;
}

bool ChessBoard::hasNextMove(int ply){
    return m_movesGen->hasNextMove(ply);
}

Move* ChessBoard::getNextMove(int ply){
    return m_movesGen->getNextMove(ply);
}

void ChessBoard::resetToFirstMove(int ply){
    m_movesGen->resetToFirstMove(ply);
}


// Generates moves until a depth p and prints the following result:
// Nb of nodes reached from the first ply nodes and time taken for a subtree search
// Depth | Nb of nodes reached | Captures | e-p | castles | promotions | check | check mates

// Finally, it checks if the zKey is correctly defined and update.

void ChessBoard::moveGenTest(uint &depth){
    this->toString();
    m_depthMax = depth;
    std::ofstream file("D:\OneDrive\Developpement\Qt\MoveGenMyEngine.txt");
    uint nodes = 0, totalNodes = 0, captures = 0, ep = 0, castles = 0, promo = 0, check = 0, checkMate = 0;
    double duration = -1;
    if(file.is_open()){
        std::clock_t start = std::clock();
        moveGenRec(&file, depth, nodes, totalNodes, captures, ep, castles, promo, check, checkMate);
        duration = (std::clock()-start)/(double) CLOCKS_PER_SEC;
    }
    file.close();
    std::cout << " ---------- RESULT -----------" << "\n";
    std::cout << "Time elapsed: " << duration << " seconds" << std::endl;
    std::cout << "Depth: " << depth << " ; Nodes: " << totalNodes << " ; Captures: " << captures << "\n";
    std::cout << "En-Passant: " << ep << " ; Castles: " << castles << " ; Promotions: " << promo << "\n";
    std::cout <<"Checks: " << check << " ; Check-mates: " << checkMate << "\n";
    std::cout << "------------------------------" << "\n";
}

void ChessBoard::moveGenRec(std::ofstream* file, uchar depth, uint& nodes, uint& totalNodes, uint& captures,
                            uint& ep, uint& castles, uint& promo, uint& check, uint& checkMate){
    if(depth > 0){
        BB possibleSquares(m_chessBB_[m_whiteIsPlaying]);

        bool isInCheck = this->generateLegalMoves(m_depthMax-depth); // set ply search
        bool isInCheckMate = isInCheck;
        Move* m;
        std::clock_t start;
        double duration;
        ZKey key;
        try{
            while(hasNextMove(m_plySearch)){
                m = getNextMove(m_plySearch);
                isInCheckMate = false;
                if(depth == 1){
                    captures += m->capture()+m->promoCapturedPiece()+m->enPassant();
                    ep += m->enPassant(), castles += m->rightCastle() | m->leftCastle(), promo += m->promotion();
                    ++nodes;
                    ++totalNodes;
                }
                key = m_zKey;
                if(depth == m_depthMax) start = std::clock();
                makeMove(m);
                moveGenRec(file, depth-1, nodes, totalNodes, captures, ep, castles, promo, check, checkMate);
                unmakeMove(m);
                if(depth == m_depthMax){
                    duration = (std::clock()-start)/(double) CLOCKS_PER_SEC;
                    std::cout << "Time elapsed: " << duration << " seconds" << std::endl;
                    if(file != NULL) *file << m->toString(false) << ": " << nodes << std::endl;
                    std::cout << m->toString(false) << ": " << nodes << std::endl;
                    nodes = 0;
                }

                if(m_zKey != key){
                    std::cout << "pb d'update de ZKey pour le move " << m->toString() << "(clé " << m_zKey << " != " << key << ")" << std::endl;
                    this->toString();
                }
                key = m_zKey;
                initZKeyFromScratch(); // change m_zKey
                if(m_zKey != key){
                    std::cout << "pb de cohérence entre la ZKeyFromScratch et la ZKey itérative pour le move " << m->toString() << std::endl;
                    this->toString();
                    m_zKey = key;
                }

                m_plySearch = m_depthMax - depth;
            }
            if(depth == 1)
                check += isInCheck, checkMate += isInCheckMate;
        }catch(EngineError& e){
            this->toString();
            std::cout << e.what() << std::endl;
        }
    }

}

// DONT USE FOR THE MOMENT
// Make the move, if possible, and store it.
void ChessBoard::makeMove(std::string move){
    /*Move* m = NoeudC::getMoveFromAlgNot(move, m_whiteIsPlaying, m_chessB);
    if(m != NULL){
        std::cout << m->toString() << std::endl;
        try{
            makeMove(m);
        }catch(EngineError e){
            std::cout << e.what() << std::endl;
        }
    }*/
}

// DONT USE FOR THE MOMENT
// Unmake the move, if possible, and delete it.
void ChessBoard::unmakeMove(std::string move){
    /*Move* m = NoeudC::getMoveFromAlgNot(move, m_whiteIsPlaying, m_chessB);
    if(!m_moves_ordered.empty() && m != NULL){
        Move* last_m = m_moves_ordered.back();
        if(*last_m == *m){
            try{
                unmakeMove(m);
            }catch(EngineError e){
                std::cout << e.what() << std::endl;
            }
            delete last_m;
        }else
            std::cout << "Not the last move\n" <<
                         "Last: " << m_moves_ordered.back()->toString() << "\n" <<
                         "Your move: " << m->toString() << std::endl;
    }else if(m_moves_ordered.empty()){
        std::cout << "No move to unmake" << std::endl;
    }
    delete m;*/
}


void ChessBoard::cleanMovesOf(const Color &c){
    BB possibleSquares(m_chessBB_[c]);
    for(Square sq = BitBoard::getIndexLeastSignBit(possibleSquares); possibleSquares > 0; possibleSquares ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(possibleSquares)){
        m_chessB[sq]->cleanMovesBB();
    }
}

void ChessBoard::playMove(Move* m){
    try{
        makeMove(m);
        m_movesGen->addMove(m);
    }catch(EngineError e){
        std::cout << "Unable to play the move" << std::endl;
        std::cout << e.what() << std::endl;
    }
}


void ChessBoard::undoMove(Move* m){
    try{
        unmakeMove(m);
        m_movesGen->deleteLastMovePlayed(m);
    }catch(EngineError e){
        std::cout << "Unable to undo the move" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

// Handle engine exception. It updates the zKey.
void ChessBoard::makeMove(Move* m) noexcept(false){
    if(m == NULL)
        throw EngineError(EngineError::MOVE_NULL_PTR_EXCEPTION);

    Square from = m->getFrom();
    Square to = m->getTo();

    if(m_chessB[from] == NULL ||
       !(m_chessBB_[m_whiteIsPlaying] & m_chessBoard & BBSquare[from]))
        throw EngineError(EngineError::CHESSBOARD_UNCONSISTANT, m->toString());

    Square enPassantSq = m->getEnPassantSq();
    BB toFrom(BBSquare[to] | BBSquare[from]);
    PieceShift pShift = (PieceShift)(m_chessB[from]->getPiece()-1);
    bool enPassant = m->enPassant();
    bool rightCastle = m->rightCastle();
    bool leftCastle = m->leftCastle();
    bool promotion = m->promotion();
    bool capture = m->capture(); // promotion capture not include
    bool doublePush = m->doublePush();
    m->pieceWasNeverUsed(m_chessB[from]->isNeverUsed());

    // On sauvegarde la place du pion qui a fait un double push.
    m->setDoublePushSq(m_doublePushSq[m_whiteIsPlaying]);
    m_doublePushSq[m_whiteIsPlaying] = (doublePush ? to : outside);

    m->setCastlingRights(m_castlingRights); // save castling rights
    m_castlingRights &= castlingRights[from][to]; // update caslting right

    updateZKey(m); // On fait l'update une fois que le move m est prêt.

    m_chessB[from]->setNeverUsed(false);

    if(enPassant | rightCastle | leftCastle | promotion){ // rare cases
        if(rightCastle){
            m_chessB[to] = m_chessB[from];  // we're moving the king
            m_chessB[to]->setSquare(to);
            m_chessB[from] = NULL;
            m_chessB[to-1] = m_chessB[to+1];// ... and the rook
            m_chessB[to-1]->setSquare((Square)(to-1));
            m_chessB[to+1] = NULL;
            m_chessBB[m_whiteIsPlaying][kingShift] = BBSquare[to];
            m_chessBB[m_whiteIsPlaying][rookShift] ^=  BBSquare[to+1] | BBSquare[to-1];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom | BBSquare[to+1] | BBSquare[to-1];
        }
        if(leftCastle){
            m_chessB[to] = m_chessB[from];  // we're moving the king
            m_chessB[to]->setSquare(to);
            m_chessB[from] = NULL;
            m_chessB[to+1] = m_chessB[to-2];// ... and the rook
            m_chessB[to+1]->setSquare((Square)(to+1));
            m_chessB[to-2] = NULL;
            m_chessBB[m_whiteIsPlaying][kingShift] = BBSquare[to];
            m_chessBB[m_whiteIsPlaying][rookShift] ^=  BBSquare[to+1] | BBSquare[to-2];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom | BBSquare[to+1] | BBSquare[to-2];
        }
        else if(enPassant){
            m_chessB[to] = m_chessB[from];
            m_chessB[to]->setSquare(to);
            m_chessB[from] = NULL;
            if(m_chessB[enPassantSq] == NULL)
                throw EngineError(EngineError::MOVE_EP_EMPTY, m->toString());
            m_deadPiecesBB[!m_whiteIsPlaying][m_indexDeadPieces[!m_whiteIsPlaying]++] = m_chessB[enPassantSq];
            m_chessB[enPassantSq] = NULL;
            m_chessBB[m_whiteIsPlaying][pawnShift] ^= toFrom;
            m_chessBB[!m_whiteIsPlaying][pawnShift] ^= BBSquare[enPassantSq];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom;
            m_chessBB_[!m_whiteIsPlaying] ^= BBSquare[enPassantSq];;
        }
        else if(promotion){
            Piece promotedPiece = m->getPromotedPiece();
            if(m_chessB[to] != NULL){
                m_deadPiecesBB[!m_whiteIsPlaying][m_indexDeadPieces[!m_whiteIsPlaying]++] = m_chessB[to];
                m_chessBB[!m_whiteIsPlaying][m_chessB[to]->getPiece()-1] ^= BBSquare[to]; // s'il y a une pièce prise, on l'enlève
                m_chessBB_[!m_whiteIsPlaying] ^= BBSquare[to];
            }
            m_chessB[to] = m_chessB[from];
            m_chessB[to]->setSquare(to);
            m_chessB[to]->setPiece(promotedPiece);
            m_chessB[from] = NULL;
            m_chessBB[m_whiteIsPlaying][pawnShift] ^= BBSquare[from];
            m_chessBB[m_whiteIsPlaying][promotedPiece-1] ^= BBSquare[to];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom;
        }
    }
    else if(capture){
        if(m_chessB[to] == NULL)
            throw EngineError(EngineError::MOVE_TO_EMPTY, m->toString());
        m_deadPiecesBB[!m_whiteIsPlaying][m_indexDeadPieces[!m_whiteIsPlaying]++] = m_chessB[to];
        PieceShift capturedPieceShift = (PieceShift)(m->getCapturedPiece()-1);
        m_chessB[to] = m_chessB[from];
        m_chessB[to]->setSquare(to);
        m_chessB[from] = NULL;
        m_chessBB[!m_whiteIsPlaying][capturedPieceShift] ^= BBSquare[to];
        m_chessBB[m_whiteIsPlaying][pShift] ^= toFrom;
        m_chessBB_[m_whiteIsPlaying] ^= toFrom;
        m_chessBB_[!m_whiteIsPlaying] ^= BBSquare[to];

    }else{
        // quiet move (no capture nor special move, double push pawn possible)
        if(m_chessB[to] != NULL){
            throw EngineError(EngineError::MOVE_TO_NOT_EMPTY, m->toString());
        }
        m_chessB[to] = m_chessB[from];
        m_chessB[to]->setSquare(to);
        m_chessB[from] = NULL;
        m_chessBB[m_whiteIsPlaying][pShift] ^= toFrom;
        m_chessBB_[m_whiteIsPlaying] ^= toFrom;
    }
    m_chessBoard = m_chessBB_[white] | m_chessBB_[black];
    m_whiteIsPlaying = (Color)(!m_whiteIsPlaying);
}

// It updates the zKey.
void ChessBoard::unmakeMove(Move* m) noexcept(false){
    if(m == NULL)
        throw EngineError(EngineError::MOVE_NULL_PTR_EXCEPTION);
    m_whiteIsPlaying = (Color)(!m_whiteIsPlaying);
    Square to = m->getFrom();
    Square from = m->getTo();
    if(m_chessB[to] != NULL || m_chessB[from] == NULL ||
      !(m_chessBB_[m_whiteIsPlaying] & m_chessBoard & BBSquare[from]))
        throw EngineError(EngineError::CHESSBOARD_UNCONSISTANT, m->toString());
    BB toFrom(BBSquare[to] | BBSquare[from]);
    PieceShift pShift = (PieceShift)(m_chessB[from]->getPiece()-1);
    Square enPassantSq = m->getEnPassantSq();

    bool enPassant = m->enPassant();
    bool rightCastle = m->rightCastle();
    bool leftCastle = m->leftCastle();
    bool promotion = m->promotion();
    bool capture = m->capture(); // promotion capture not include

    if(enPassant | rightCastle | leftCastle | promotion){ // rare cases
        if(rightCastle){
            m_chessB[to] = m_chessB[from];  // we're moving the king
            m_chessB[to]->setSquare(to);
            m_chessB[from] = NULL;
            m_chessB[from+1] = m_chessB[from-1];// ... and the rook
            m_chessB[from+1]->setSquare((Square)(from+1));
            m_chessB[from-1] = NULL;
            m_chessBB[m_whiteIsPlaying][kingShift] = BBSquare[to];
            m_chessBB[m_whiteIsPlaying][rookShift] ^=  BBSquare[from+1] | BBSquare[from-1];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom | BBSquare[from+1] | BBSquare[from-1];
        }
        if(leftCastle){
            m_chessB[to] = m_chessB[from];  // we're moving the king
            m_chessB[to]->setSquare(to);
            m_chessB[from] = NULL;
            m_chessB[from-2] = m_chessB[from+1];// ... and the rook
            m_chessB[from-2]->setSquare((Square)(from-2));
            m_chessB[from+1] = NULL;
            m_chessBB[m_whiteIsPlaying][kingShift] = BBSquare[to];
            m_chessBB[m_whiteIsPlaying][rookShift] ^=  BBSquare[from+1] | BBSquare[from-2];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom | BBSquare[from+1] | BBSquare[from-2];
        }
        else if(enPassant){
            m_chessB[to] = m_chessB[from];
            m_chessB[to]->setSquare(to);
            m_chessB[from] = NULL;
            m_chessB[enPassantSq] = m_deadPiecesBB[!m_whiteIsPlaying][--m_indexDeadPieces[!m_whiteIsPlaying]];
            if(m_chessB[enPassantSq] == NULL)
                throw EngineError(EngineError::MOVE_EP_EMPTY, m->toString());
            m_deadPiecesBB[!m_whiteIsPlaying][m_indexDeadPieces[!m_whiteIsPlaying]] = NULL;
            m_chessBB[m_whiteIsPlaying][pawnShift] ^= toFrom;
            m_chessBB[!m_whiteIsPlaying][pawnShift] ^= BBSquare[enPassantSq];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom;
            m_chessBB_[!m_whiteIsPlaying] ^= BBSquare[enPassantSq];
        }
        else if(promotion){
            Piece promotedPiece = m->getPromotedPiece();
            Piece capturedPiece = m->getPromoCapturedPiece();
            m_chessB[to] = m_chessB[from];
            if(capturedPiece){
                m_chessB[from] = m_deadPiecesBB[!m_whiteIsPlaying][--m_indexDeadPieces[!m_whiteIsPlaying]];
                if(m_chessB[from] == NULL)
                    throw EngineError(EngineError::MOVE_EMPTY_CAPTURE, m->toString());
                m_deadPiecesBB[!m_whiteIsPlaying][m_indexDeadPieces[!m_whiteIsPlaying]] = NULL;
                m_chessBB[!m_whiteIsPlaying][capturedPiece-1] ^= BBSquare[from];
                m_chessBB_[!m_whiteIsPlaying] ^= BBSquare[from];
            }else{
                m_chessB[from] = NULL;
            }
            m_chessB[to]->setSquare(to);
            m_chessB[to]->setPiece(pawn);
            m_chessBB[m_whiteIsPlaying][pawnShift] ^= BBSquare[to];
            m_chessBB[m_whiteIsPlaying][promotedPiece - 1] ^= BBSquare[from];
            m_chessBB_[m_whiteIsPlaying] ^= toFrom;
        }
    }
    else if(capture){
        PieceShift capturedPieceShift = (PieceShift)(m->getCapturedPiece()-1);
        m_chessB[to] = m_chessB[from];
        m_chessB[from] = m_deadPiecesBB[!m_whiteIsPlaying][--m_indexDeadPieces[!m_whiteIsPlaying]];
        if(m_chessB[from] == NULL)
            throw EngineError(EngineError::MOVE_EMPTY_CAPTURE, m->toString());
        m_deadPiecesBB[!m_whiteIsPlaying][m_indexDeadPieces[!m_whiteIsPlaying]] = NULL;
        m_chessB[to]->setSquare(to);
        m_chessBB[!m_whiteIsPlaying][capturedPieceShift] ^= BBSquare[from];
        m_chessBB[m_whiteIsPlaying][pShift] ^= toFrom;
        m_chessBB_[m_whiteIsPlaying] ^= toFrom;
        m_chessBB_[!m_whiteIsPlaying] ^= BBSquare[from];

    }else{
        // quiet move (no capture nor special move, double push pawn possible)
        m_chessB[to] = m_chessB[from];
        m_chessB[to]->setSquare(to);
        m_chessB[from] = NULL;
        m_chessBB[m_whiteIsPlaying][pShift] ^= toFrom;
        m_chessBB_[m_whiteIsPlaying] ^= toFrom;
    }

    updateZKey(m);
    m_chessB[to]->setNeverUsed(m->wasPieceNeverUsed());

    m_doublePushSq[m_whiteIsPlaying] = m->getDoublePushSq();
    m_castlingRights = m->getCastlingRights(); // restore castling rights

    m_chessBoard = m_chessBB_[white] | m_chessBB_[black];
}


void ChessBoard::generateVectorizedMoves(NoeudC* node,
                                             const bool& rightCastle,
                                             const bool& leftCastle,
                                             const bool& enPassant,
                                             const bool& promotion,
                                             const bool& doublePush){
    // On suppose que le BB moves de la pièce est à jour
    BB moves(node->getMovesBB()); // cp of moves.

    Square from = node->getSquare();
    Square to = outside;
    // All these tests are usually false.
    if(rightCastle | leftCastle | enPassant | promotion){

        if(rightCastle){
            m_movesGen->addToPly(m_plySearch, from, (Square)(from+2), RIGHT_CASTLE_FLAG);
            moves ^= BBSquare[from+2];
        }
        if(leftCastle){
            m_movesGen->addToPly(m_plySearch, from,(Square)(from-2), LEFT_CASTLE_FLAG);
            moves ^= BBSquare[from-2];
        }
        else if(enPassant){
            Square enPassantSq((Square)(m_doublePushSq[!m_whiteIsPlaying] + doublePushEPSq[!m_whiteIsPlaying]));
           m_movesGen->addToPly(m_plySearch, from, enPassantSq, EN_PASSANT_FLAG);
            moves ^= BBSquare[enPassantSq];
        }
        else if(promotion){
            BB promotionSquares(pawnPromotionRaw[m_whiteIsPlaying] & moves);
            moves ^= promotionSquares;
            for(to = BitBoard::getIndexLeastSignBit(promotionSquares); promotionSquares > 0; promotionSquares ^= BBSquare[to], to = BitBoard::getIndexLeastSignBit(promotionSquares)){ // generally 1-2 sq (for a promotion: 4 possible moves)
                for(Piece promotionKind = rook; promotionKind < king; promotionKind = (Piece)(promotionKind+1)){
                     m_movesGen->addToPly(m_plySearch, from, to, PROMOTION_FLAG[m_chessB[to] == NULL ? empty : m_chessB[to]->getPiece()][promotionKind-rook]);
                }
            }
        }
    }else if(doublePush){
        m_movesGen->addToPly(m_plySearch, from, (Square)(from+doublePushPawn[m_whiteIsPlaying]), DOUBLE_PUSH_FLAG);
        moves ^= BBSquare[from+doublePushPawn[m_whiteIsPlaying]];
    }

    for(to = BitBoard::getIndexLeastSignBit(moves); moves>0; moves ^= BBSquare[to], to = BitBoard::getIndexLeastSignBit(moves)){
        // OPTION DE DEBUGGAGE
        if(m_chessB[to] != NULL && m_chessB[to]->getPiece() == king) // veut dire que les pinned pieces sont fausses.
            throw EngineError(EngineError::CHESSBOARD_UNCONSISTANT);
         m_movesGen->addToPly(m_plySearch, from, to, CAPTURE_FLAG[m_chessB[to] == NULL ? empty : m_chessB[to]->getPiece()]);
    }
}

// returns true if the king is attacked (check-mate or check)
bool ChessBoard::generateLegalMoves(int ply){
    m_plySearch = ply;
    m_movesGen->cleanMovesForPly(m_plySearch);
    cleanMovesOf(m_whiteIsPlaying);

    BB pinnedAllyPieces = 0, attackers = 0, enPassantAttack = 0, enPassantBlocker = 0, possibleSquares = 0, piecesByKind = 0;
    bool rightCastle = false, leftCastle = false, enPassantF = false, promotion = false, doublePushPawn = false;
    const BB notOurPieces(~m_chessBB_[m_whiteIsPlaying]);
    Square sq = BitBoard::getIndexLeastSignBit(m_chessBB[m_whiteIsPlaying][kingShift]);
    NoeudC* king = m_chessB[sq];
    Square sqE = outside;

    analysePiecesAdjAvecClouage(sq, m_whiteIsPlaying, pinnedAllyPieces, attackers);
    bool kingIsAttacked = (attackers > 0);
    possibleSquares = kingAttacks[sq] & notOurPieces;

    for(sqE = BitBoard::getIndexLeastSignBit(possibleSquares); possibleSquares > 0; possibleSquares ^= BBSquare[sqE], sqE = BitBoard::getIndexLeastSignBit(possibleSquares)){ // king (max 8 possible squares)
        king->addMove(indicatrice[!squareIsAttacked(sqE, (Color)!m_whiteIsPlaying)] & BBSquare[sqE]);
    }
    if(kingIsAttacked){
        generateVectorizedMoves(king,  rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
        Square attacker = BitBoard::getIndexLeastSignBit(attackers);
        if(!(attackers ^ BBSquare[attacker])){
            // On ne garde que les pions non cloués.
            // Si un pion est cloué et qu'il y a une pièce qui met en échec le roi, ce n'est pas
            // celle qui cloue le pion. Donc celui-ci ne peut rien faire !
            piecesByKind = m_chessBB[m_whiteIsPlaying][pawnShift]  & ~pinnedAllyPieces;
            BB inBetweenKingAttacker(indicatrice[(attackers & m_chessBB[!m_whiteIsPlaying][knightShift]) == 0] & (inBetweenArray[sq][attacker] ^ attackers));
            BB enPassantEaters(indicatrice[(attackers & BBSquare[m_doublePushSq[!m_whiteIsPlaying]])>0] & HAdjacent[attacker] & piecesByKind);
            piecesByKind ^= enPassantEaters; // on enlève les pions qui peuvent prendre en passant.
            // Même si un pion n'est pas marqué "pinned", il peut y avoir le cas rare d'un pion qui possède un 'pinnedMovesBB' =! 0
            // Il faut donc nettoyer systématiquement tous pions. Les autres pièces n'en ont pas besoin en théorie.
            // (*)


            for(sq = BitBoard::getIndexLeastSignBit(enPassantEaters); enPassantEaters > 0; enPassantEaters ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(enPassantEaters)){
                m_chessB[sq]->setMove(pawnMove[m_whiteIsPlaying][attacker] & ~m_chessBoard);
                m_chessB[sq]->cleanPinnedMovesBB(); // cf (*)
                generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, m_chessB[sq]->hasNonEmptyMove(), promotion, doublePushPawn);
            }
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
                // pawns (max 8 pawns)
                possibleSquares = (indicatrice[(pawnPartialMove[m_whiteIsPlaying][sq] & ~m_chessBoard) > 0] &
                                   pawnMove[m_whiteIsPlaying][sq] & inBetweenKingAttacker) | // blocker
                                  (pawnAttacks[m_whiteIsPlaying][sq] & attackers);           // eater
                m_chessB[sq]->setMove(possibleSquares);
                doublePushPawn = (possibleSquares & (pawnMove[m_whiteIsPlaying][sq] ^ pawnPartialMove[m_whiteIsPlaying][sq])) > 0;
                promotion = (pawnPromotionRaw[m_whiteIsPlaying] & possibleSquares) > 0;
                enPassantBlocker = pawnAttacks[m_whiteIsPlaying][sq] & inBetweenKingAttacker;
                if(enPassantBlocker){  // really rare case : intersection between attack of our pawn and king-attacker
                    enPassantAttack = m_chessBB[!m_whiteIsPlaying][pawnShift] & pawnPartialMove[!m_whiteIsPlaying][BitBoard::getIndexLeastSignBit(enPassantBlocker)];
                    if(enPassantAttack){ // even more rare : horizontal adjacent ennemy
                        if(enPassantAttack & BBSquare[m_doublePushSq[!m_whiteIsPlaying]]){
                            m_chessB[sq]->addMove(enPassantBlocker);
                            enPassantF = true;
                        }
                    }
                }
                m_chessB[sq]->cleanPinnedMovesBB(); // cf (*)
                generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
                doublePushPawn = false, enPassantF = false, promotion = false;
            }
            piecesByKind = m_chessBB[m_whiteIsPlaying][pawnShift] & pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind))
                m_chessB[sq]->cleanPinnedMovesBB();

            // On ne prend pas les pièces pinned et on nettoie après celle qui l'ont été.
            piecesByKind = m_chessBB[m_whiteIsPlaying][knightShift] & ~pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
                // knigths (max 2 knights)
                m_chessB[sq]->setMove(knightAttacks[sq] & (inBetweenKingAttacker | attackers)); // eater and blocker
                generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
            }
            piecesByKind = m_chessBB[m_whiteIsPlaying][knightShift] & pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind))
                m_chessB[sq]->cleanPinnedMovesBB();

            piecesByKind = m_chessBB[m_whiteIsPlaying][bishopShift] & ~pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
             // bishop pieces (max 2 pieces)
                bishopAttacksBB(sq, possibleSquares);
                possibleSquares &= (inBetweenKingAttacker | attackers);
                m_chessB[sq]->setMove(possibleSquares);
                generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
            }
            piecesByKind = m_chessBB[m_whiteIsPlaying][bishopShift] & pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind))
                m_chessB[sq]->cleanPinnedMovesBB();


            piecesByKind = m_chessBB[m_whiteIsPlaying][rookShift] & ~pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
             // rook pieces (max 2 pieces)
                rookAttacksBB(sq, possibleSquares);
                possibleSquares &= (inBetweenKingAttacker | attackers);
                m_chessB[sq]->setMove(possibleSquares);
                generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
            }
            piecesByKind = m_chessBB[m_whiteIsPlaying][rookShift] & pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind))
                m_chessB[sq]->cleanPinnedMovesBB();


            piecesByKind = m_chessBB[m_whiteIsPlaying][queenShift] & ~pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
              // queen pieces (max 1 piece)
                queenAttacksBB(sq, possibleSquares);
                possibleSquares &= (inBetweenKingAttacker | attackers);
                m_chessB[sq]->setMove(possibleSquares);
                generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
            }
            piecesByKind = m_chessBB[m_whiteIsPlaying][queenShift] & pinnedAllyPieces;
            for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind))
                m_chessB[sq]->cleanPinnedMovesBB();


        }
    }else{ // king's not in check: castling is possible

        // Castling special case (rare case)
        if(king->isNeverUsed()){
            if(!(petitRoque[m_whiteIsPlaying] & m_chessBoard)){ // king not used and free space for little castling
                NoeudC* rightRook = m_chessB[rightRookByColor[m_whiteIsPlaying]];
                if(rightRook != NULL && rightRook->isNeverUsed() && ((m_chessBB[m_whiteIsPlaying][kingShift] << 1) & king->getMovesBB())){
                    // right rook never used and first king's adjacent square not attacked
                    if(!squareIsAttacked((Square)(king->getSquare()+2), (Color)!m_whiteIsPlaying)){
                        king->addMove(BBSquare[king->getSquare()+2]);
                        rightCastle = true;
                    }
                }
            }
            if(!(grandRoque[m_whiteIsPlaying] & m_chessBoard)){ // king not used and free space for big castling
                NoeudC* leftRook = m_chessB[leftRookByColor[m_whiteIsPlaying]];
                if(leftRook != NULL && leftRook->isNeverUsed() && ((m_chessBB[m_whiteIsPlaying][kingShift] >> 1) & king->getMovesBB())){
                    // left rook never used and first king's adjacent square not attacked
                    if(!squareIsAttacked((Square)(king->getSquare()-2), (Color)!m_whiteIsPlaying)){
                        king->addMove(BBSquare[king->getSquare()-2]);
                        leftCastle = true;
                    }
                }
            }
            generateVectorizedMoves(king, rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
            rightCastle = false, leftCastle = false;
        } else {
            generateVectorizedMoves(king, rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
        }
        piecesByKind = m_chessBB[m_whiteIsPlaying][pawnShift];
        for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
         // pawns
            possibleSquares = (indicatrice[(pawnPartialMove[m_whiteIsPlaying][sq] & ~m_chessBoard)>0] &
                              pawnMove[m_whiteIsPlaying][sq] & ~m_chessBoard) | // usual move
                              (pawnAttacks[m_whiteIsPlaying][sq] & m_chessBB_[!m_whiteIsPlaying]); // eater
            enPassantAttack = m_chessBB[!m_whiteIsPlaying][pawnShift] & HAdjacent[sq] & BBSquare[m_doublePushSq[!m_whiteIsPlaying]]; // max 1 pieces
            if(enPassantAttack){
                enPassantBlocker = pawnPartialMove[m_whiteIsPlaying][BitBoard::getIndexLeastSignBit(enPassantAttack)] & ~m_chessBoard;
                possibleSquares |= enPassantBlocker;
            }
            m_chessB[sq]->andMove(possibleSquares); // mise à jour possible de possibleSquares !!
            if(possibleSquares & m_chessBB[!m_whiteIsPlaying][kingShift]){ // vérif qu'un pion ne mange pas le roi adverse
                throw EngineError(EngineError::CHESSBOARD_UNCONSISTANT);
            }
            m_chessB[sq]->cleanPinnedMovesBB();
            doublePushPawn = (possibleSquares & (pawnMove[m_whiteIsPlaying][sq] ^ pawnPartialMove[m_whiteIsPlaying][sq])) > 0;
            promotion = (pawnPromotionRaw[m_whiteIsPlaying] & possibleSquares) > 0;
            enPassantF = (enPassantBlocker & possibleSquares) > 0; // peut-être ne peut-on plus faire la prise en passant.

            generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
            enPassantF= false, doublePushPawn = false, promotion = false;
        }
        piecesByKind = m_chessBB[m_whiteIsPlaying][knightShift] & ~pinnedAllyPieces;
        for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
         // knigths
            m_chessB[sq]->setMove(knightAttacks[sq] & notOurPieces);
            generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
        }
        piecesByKind = m_chessBB[m_whiteIsPlaying][bishopShift];
        for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
         // bishops
            bishopAttacksBB(sq, possibleSquares);
            possibleSquares &= notOurPieces;
            m_chessB[sq]->andMove(possibleSquares);
            m_chessB[sq]->cleanPinnedMovesBB();
            generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
        }
        piecesByKind = m_chessBB[m_whiteIsPlaying][rookShift];
        for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
         // rooks
            rookAttacksBB(sq, possibleSquares);
            possibleSquares &= notOurPieces;
            m_chessB[sq]->andMove(possibleSquares);
            m_chessB[sq]->cleanPinnedMovesBB();
            generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
        }
        piecesByKind = m_chessBB[m_whiteIsPlaying][queenShift];
        for(sq = BitBoard::getIndexLeastSignBit(piecesByKind); piecesByKind > 0; piecesByKind ^= BBSquare[sq], sq = BitBoard::getIndexLeastSignBit(piecesByKind)){
          // queen
            queenAttacksBB(sq, possibleSquares);
            possibleSquares &= notOurPieces;
            m_chessB[sq]->andMove(possibleSquares);
            m_chessB[sq]->cleanPinnedMovesBB();
            generateVectorizedMoves(m_chessB[sq], rightCastle, leftCastle, enPassantF, promotion, doublePushPawn);
        }
    }
    return kingIsAttacked;
}

void ChessBoard::bishopAttacksBB(Square sq, BB& result){
    result = (((m_chessBoard & diagAttacks[sq]) * firstFileMask) >> 56);
    BB resultADiag(((m_chessBoard & antiDiagAttacks[sq]) * firstFileMask) >> 56);
    result = fillCopyOfLastRow[byteFillingOccupancy[result & 255][sq & 7]] & diagAttacks[sq];
    resultADiag = fillCopyOfLastRow[byteFillingOccupancy[resultADiag & 255][sq & 7]] & antiDiagAttacks[sq];
    result |= resultADiag;
}

void ChessBoard::rookAttacksBB(Square sq, BB& result){
   uchar shift = 8*(sq/8);
   result = m_chessBoard & HAttacks[sq];
   result = byteFillingOccupancy[(result >> shift) & 255][sq & 7];
   result <<= shift;
   BB resultV((((m_chessBoard & VAttacks[sq]) >> (sq & 7)) * diagMask) >> 56);
   resultV = fillCopyOfLastFile[byteFillingOccupancy[resultV & 255][7-sq/8]] & VAttacks[sq];
   result |= resultV;
}

void ChessBoard::queenAttacksBB(Square sq, BB &result){
    BB resultRook;
    rookAttacksBB(sq, resultRook);
    bishopAttacksBB(sq, result);
    result |= resultRook;

}

// Color c : color of our ennemy pieces
// Square sq: square to check
bool ChessBoard::squareIsAttacked(Square sq, Color c){
    if(pawnAttacks[1^c][sq] & m_chessBB[c][pawnShift])
        return true;
    else if(knightAttacks[sq] & m_chessBB[c][knightShift])
        return true;
    else if(kingAttacks[sq] & m_chessBB[c][kingShift])
        return true;
    else{
        // max 5 attackers
        BB attackers((bishopAttacks[sq] & (m_chessBB[c][bishopShift] | m_chessBB[c][queenShift])) |
                       (rookAttacks[sq] & (m_chessBB[c][rookShift] | m_chessBB[c][queenShift])));
        BB chessBoardWithoutKing(m_chessBoard ^ m_chessBB[1^c][kingShift]);
        BB squareAttackedBB(BBSquare[sq]);
        Square attacker;
        for(attacker = BitBoard::getIndexLeastSignBit(attackers); attackers > 0; attackers ^= BBSquare[attacker], attacker = BitBoard::getIndexLeastSignBit(attackers)){
            if(((squareAttackedBB ^ inBetweenArray[attacker][sq]) & chessBoardWithoutKing) == 0)
                return true;
        }
    }
    return false;
}

// Color c: color of our ally pieces
// Analyse dans toutes les directions depuis "sq"
void ChessBoard::analysePiecesAdjAvecClouage(Square sq, Color c, BB& pinnedAllyPieces, BB& ennemyAdjBB){

    // D'abord, on regarde les pions et cavaliers adverses.
    ennemyAdjBB = pawnAttacks[c][sq] & m_chessBB[1^c][pawnShift] |
                  knightAttacks[sq] & m_chessBB[1^c][knightShift];
    // Ensuite on sépare l'analyse par direction.
    BB diagEnnemyPieces(m_chessBB[1^c][queenShift] | m_chessBB[1^c][bishopShift]);
    BB fileEnnemyPieces(m_chessBB[1^c][queenShift] | m_chessBB[1^c][rookShift]);
    Square sqAlly;

    //diag
    BB result(diagAttacks[sq] & m_chessBoard);
    BitBoard::diagClosestBitsFromSq(diagAttacks[sq], result, sq);
    ennemyAdjBB |= result & diagEnnemyPieces;
    BB allyAdjBB(result & m_chessBB_[c]);

    for(sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB); allyAdjBB>0; allyAdjBB ^= BBSquare[sqAlly], sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB)){
        // max 2 pieces
        result = diagAttacks[sqAlly] & m_chessBoard;
        BitBoard::diagClosestBitsFromSq(diagAttacks[sqAlly], result, sqAlly);
        result &= diagEnnemyPieces;
        if(result){ // max one piece
            m_chessB[sqAlly]->setPinnedMove(inBetweenArray[sq][BitBoard::getIndexLeastSignBit(result)]);
            pinnedAllyPieces = BBSquare[sqAlly];
        }
    }
    // anti-diag
    result = antiDiagAttacks[sq] & m_chessBoard;
    BitBoard::antiDiagClosestBitsFromSq(antiDiagAttacks[sq], result, sq);
    ennemyAdjBB |= result & diagEnnemyPieces;
    allyAdjBB = result & m_chessBB_[c];
    for(sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB); allyAdjBB>0; allyAdjBB ^=BBSquare[sqAlly], sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB)){
        result = antiDiagAttacks[sqAlly] & m_chessBoard;
        BitBoard::diagClosestBitsFromSq(antiDiagAttacks[sqAlly], result, sqAlly);
        result &= diagEnnemyPieces;
        if(result){ // max one piece
            m_chessB[sqAlly]->setPinnedMove(inBetweenArray[sq][BitBoard::getIndexLeastSignBit(result)]);
            pinnedAllyPieces |= BBSquare[sqAlly];
        }
    }

    //horizontal
    result = HAttacks[sq] & m_chessBoard;
    BitBoard::rankClosestBitsFromSq(result, sq);
    ennemyAdjBB |= result & fileEnnemyPieces;
    BB EPPawnAdj(result & (m_chessBB[black][pawnShift] | m_chessBB[white][pawnShift]));
    allyAdjBB = result & m_chessBB_[c];
    for(sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB); allyAdjBB>0; allyAdjBB ^= BBSquare[sqAlly], sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB)){
        result = HAttacks[sqAlly] & m_chessBoard;
        BitBoard::rankClosestBitsFromSq(result, sqAlly);
        result &= fileEnnemyPieces;
        if(result){ // max one piece
            m_chessB[sqAlly]->setPinnedMove(inBetweenArray[sq][BitBoard::getIndexLeastSignBit(result)]);
            pinnedAllyPieces |= BBSquare[sqAlly];
        }
    }

    // vertical
    result = VAttacks[sq] & m_chessBoard;
    BitBoard::fileClosestBitsFromSq(VAttacks[sq], result, sq);
    ennemyAdjBB |= result & fileEnnemyPieces;
    allyAdjBB = result & m_chessBB_[c];
    for(sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB); allyAdjBB>0; allyAdjBB ^= BBSquare[sqAlly], sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB)){
        sqAlly = BitBoard::getIndexLeastSignBit(allyAdjBB);
        result = VAttacks[sqAlly] & m_chessBoard;
        BitBoard::fileClosestBitsFromSq(VAttacks[sq], result, sqAlly);
        result &= fileEnnemyPieces;
        if(result){ // max one piece
            m_chessB[sqAlly]->setPinnedMove(inBetweenArray[sq][BitBoard::getIndexLeastSignBit(result)]);
            pinnedAllyPieces |= BBSquare[sqAlly];
        }
    }


    // EPPawnAdj : le pion (blanc ou noir) adjacent horizontalement au roi
    // Vérif: si c'est un pion de notre couleur, on vérifie que le pion
    // à sa droite ou sa gauche peut être pris en passant. Si ce n'est pas
    // notre pion, on vérifie qu'il peut être pris EP et qu'il y a un pion
    // voisin de notre couleur

    if(EPPawnAdj & m_chessBB[c][pawnShift]){
        // pion de notre couleur
        sqAlly = BitBoard::getIndexLeastSignBit(EPPawnAdj);
        Square sqEnnemy = BitBoard::getIndexLeastSignBit(HAdjacent[sqAlly] & BBSquare[m_doublePushSq[1^c]]);
        if(sqEnnemy<64 && (pawnPartialMove[c][sqEnnemy] & ~m_chessBoard)){
            // on peut prendre un pion en passant. Vérif de la mise en échec
            result = HAttacks[sqEnnemy] & m_chessBoard; // ligne associée au pion pris e-p ennemi.
            BitBoard::rankClosestBitsFromSq(result, sqAlly);
            if(result & fileEnnemyPieces){
                // on peut être attacké: on enlève ce déplacement de pion
                m_chessB[sqAlly]->setPinnedMove(~pawnPartialMove[c][sqEnnemy]);
                //std::cout << "Prise en passant interdite" << std::endl;
                //BitBoard::toString(pawnPartialMove[c][sqEnnemy]);
                //this->toString();
            }
        }
    }else if(EPPawnAdj & m_chessBB[1^c][pawnShift]){
        // pion adverse
        Square sqEnnemy = BitBoard::getIndexLeastSignBit(EPPawnAdj & BBSquare[m_doublePushSq[1^c]]);
        sqAlly = BitBoard::getIndexLeastSignBit(HAdjacent[sqEnnemy] & m_chessBB[c][pawnShift]);
        if(sqAlly<64 && (pawnPartialMove[c][sqEnnemy] & ~m_chessBoard)){
            // on peut prendre le pion en passant. Vérif si mise en échec
            result = HAttacks[sqAlly] & m_chessBoard; // ligne associée au pion pris e-p allié.
            BitBoard::rankClosestBitsFromSq(result, sqEnnemy);
            if(result & fileEnnemyPieces){
                // on peut attacker notre roi.
                m_chessB[sqAlly]->setPinnedMove(~pawnPartialMove[c][sqEnnemy]);
                //std::cout << "Prise en passant interdite" << std::endl;
                //BitBoard::toString(pawnPartialMove[c][sqEnnemy]);
                //this->toString();
            }
        }
    }
}

// Application involutive: f o f = id (car (x^y)^y = x)
// Color of the side to move: m_whiteIsPlaying
// We assume that the move's going to be played (or it has been unmade).

// If it's not a special move (no promo, capture, ep, castles), it
// takes 5 or 7 xor between 64 bits integers.

bool ChessBoard::updateZKey(Move* m){
    if(m == NULL){
        std::cout << "Warning: updating ZKey with null move." << std::endl;
        return false;
    }
    if(m->getFrom() >= outside || m->getFrom() < 0 || m_chessB[m->getFrom()] == NULL){
        std::cout << "No piece at the start position. Impossible to update ZKey." << std::endl;
        return false;
    }
    // Redef side to move
    m_zKey ^= ZobristHash::CHANGING_SIDE;

    // Redef the ep square
    if(m_doublePushSq[m_whiteIsPlaying] != outside)
        m_zKey ^= ZobristHash::EP_SQ_FILE[m_doublePushSq[m_whiteIsPlaying]%8];
    if(m_doublePushSq[!m_whiteIsPlaying] != outside)
        m_zKey ^=ZobristHash::EP_SQ_FILE[m_doublePushSq[!m_whiteIsPlaying]%8];

    NoeudC* myPiece = m_chessB[m->getFrom()];

    // If promotion, tricky situation
    if(m->promotion()) {
        if(m->getPromoCapturedPiece()) // remove the ennemy piece
            m_zKey ^= ZobristHash::PIECE_HASH[!m_whiteIsPlaying][m->getPromoCapturedPiece()][m->getTo()];
        // Important: replace the promoted pawn
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][pawn][m->getFrom()];
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][m->getPromotedPiece()][m->getTo()];
    }else {
        // Redef the position of our piece
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][myPiece->getPiece()][m->getFrom()];
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][myPiece->getPiece()][m->getTo()];
    }

    // If capture, remove the ennemy piece
    if(m->capture())
        m_zKey ^= ZobristHash::PIECE_HASH[!m_whiteIsPlaying][m->getCapturedPiece()][m->getTo()];

    // If EP capture, remove the ennemy pawn
    if(m->enPassant())
        m_zKey ^= ZobristHash::PIECE_HASH[!m_whiteIsPlaying][pawn][m->getEnPassantSq()];

    // If right castle, we move the rook.
    if(m->rightCastle()){
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][rook][rightRookByColor[m_whiteIsPlaying]];
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][rook][m->getFrom()+1];
    }
    // idem
    if(m->leftCastle()){
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][rook][leftRookByColor[m_whiteIsPlaying]];
        m_zKey ^= ZobristHash::PIECE_HASH[m_whiteIsPlaying][rook][m->getFrom()-1];
    }

    // Handle right castling.
    if(m_castlingRights != m->getCastlingRights()){
        // On prend seulement les 4 premiers bits
        m_zKey ^= ZobristHash::CASTLING_RIGHTS[m->getCastlingRights()&15]; // On enlève les droits précédents
        m_zKey ^= ZobristHash::CASTLING_RIGHTS[m_castlingRights&15];       // On met les nouveaux droits
    }

    return true;
}

// Initialize the zobrist key of our chessboard. Returns false if errors.
bool ChessBoard::initZKeyFromScratch(){
    m_zKey = 0;
    // First: iterate over all the pieces
    for(uchar sq = 0; sq < 64; ++sq){
        if(m_chessB[sq] != NULL){
            m_zKey ^= ZobristHash::PIECE_HASH[m_chessB[sq]->getColor()][m_chessB[sq]->getPiece()][m_chessB[sq]->getSquare()];
        }
    }
    // Then the side to move
    m_zKey ^= ZobristHash::getColorHash(m_whiteIsPlaying);

    // Then, the EP sq
    m_zKey ^= ZobristHash::getEPFileSqHash(m_doublePushSq[!m_whiteIsPlaying]);

    // Finally the castles
    CRights cR = 0;
    for(uchar c = black; c < 2; ++c){
        if(m_chessB[kingInitSqByColor[c]] != NULL && m_chessB[kingInitSqByColor[c]]->isNeverUsed()){
            if(m_chessB[rightRookByColor[c]] != NULL && m_chessB[rightRookByColor[c]]->isNeverUsed()){
                cR |= ZobristHash::RIGHT_CASTLE[c];
            }
            if(m_chessB[leftRookByColor[c]] != NULL && m_chessB[leftRookByColor[c]]->isNeverUsed()){
                cR |= ZobristHash::LEFT_CASTLE[c];
            }
        }
    }
    m_zKey ^= ZobristHash::CASTLING_RIGHTS[cR&15];
    return true;
}


void ChessBoard::toString(){

    BB mask(1);
    std::stringstream result;
    result << "\n ################################";
    std::string piece ="";
    char lineNb = '8';
    for(int i = 0; i < 64 ; ++i){
        if(i%8 == 0){
            result << "\n" << lineNb;
            lineNb -= 1;
        }
        if(mask & m_chessBB_[black])
            piece = "n ";
        else if(mask & m_chessBB_[white])
            piece = "b ";
        else
            piece = "  ";
        if(mask & (m_chessBB[black][pawnShift] | m_chessBB[white][pawnShift]))
            piece = " P" + piece;
        else if(mask & (m_chessBB[black][rookShift] | m_chessBB[white][rookShift]))
            piece = " T" + piece;
        else if(mask & (m_chessBB[black][bishopShift] | m_chessBB[white][bishopShift]))
            piece = " F" + piece;
        else if(mask & (m_chessBB[black][knightShift] | m_chessBB[white][knightShift]))
            piece = " C" + piece;
        else if(mask & (m_chessBB[black][queenShift] | m_chessBB[white][queenShift]))
            piece = " D" + piece;
        else if(mask & (m_chessBB[black][kingShift] | m_chessBB[white][kingShift]))
            piece = " R" + piece;
        else
            piece = "  " + piece;
        result << piece;
        mask <<= 1;
    }
    result << " \n  a   b   c   d   e   f   g   h\n";
    std::cout << result.str();
}

// Getters and destructors

ZKey ChessBoard::getZobristKey() const{
    return m_zKey;
}
CRights ChessBoard::getCastlingRights() const{
    return m_castlingRights;
}
BB ChessBoard::getChessBB() const{
    return m_chessBoard;
}
Color ChessBoard::getColorToPlay() const{
    return m_whiteIsPlaying;
}
int ChessBoard::getPlySearch() const{
    return m_plySearch;
}
int ChessBoard::getMaxDepth() const{
    return m_depthMax;
}
MovesGenerator *ChessBoard::getMovesGen() const{
    return m_movesGen;
}
int ChessBoard::getIndexDeadPieces(Color c) const{
    return m_indexDeadPieces[c];
}
BB ChessBoard::getChessBBPerColor(Color c) const{
    return m_chessBB_[c];
}
Square ChessBoard::getDoublePushSq(Color c) const{
    return m_doublePushSq[c];
}
BB ChessBoard::getChessBBPerColorAndKind(Color c, PieceShift kind) const{
    return m_chessBB[c][kind];
}
NoeudC* ChessBoard::getDeadPiece(Color c, int k) const{
    if(k < 0 || k >= 16){
        std::cout << "getDeadPiece of index " << k << ". Returned null" << std::endl;
        return NULL;
    }
    return m_deadPiecesBB[c][k];
}
NoeudC* ChessBoard::getChessBPiece(Square sq) const{
    return m_chessB[sq];
}

ChessBoard::~ChessBoard(){
    for(uchar sq = 0; sq < 64; ++sq){
        delete m_chessB[sq];
    }
    for(uchar sq = 0; sq < 16; ++sq){
        delete m_deadPiecesBB[white][sq];
        delete m_deadPiecesBB[black][sq];
    }
    delete m_movesGen;
}
