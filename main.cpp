#include<chessboard.h>
#include<QCoreApplication>

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    ChessBoard chessBoard;
    if(chessBoard.initChessBoard()){
        uint depth = 1;
        chessBoard.moveGenTest(depth);
    }
    return 0;
}

