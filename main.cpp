#include<ia.h>
#include<QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ChessBoard chessBoard;

    if(chessBoard.initChessBoard()){
        std::cout <<"Hello worlds" << std::endl;
        uint depth = 7;
        chessBoard.toString();
        //IA ia(&chessBoard, depth);
        //ia.playAGame(10);
        chessBoard.moveGenTest(depth);
    }
    return a.exec();
}

