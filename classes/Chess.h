#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"

constexpr int pieceSize = 80;


class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;
    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    void stopGame() override;
    Player *checkForWinner() override;
    bool checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    Grid* getGrid() override { return _grid; }

    void indexToSquare(int index, int &x, int &y);
    int squareToIndex(int x, int y);
    void generateKnightMoveBitboards();
    void generateKingMoveBitboards();
    std::vector<int> generatePawnMovesFrom(int from, bool isWhite);
    void generateAllBitboards();
    std::vector<BitMove> generateAllMoves(int square);
    std::vector<BitMove> generatePawnMoves(int square, bool isWhite);

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;
    BitboardElement _whitePawns;
    BitboardElement _whiteKnights;
    BitboardElement _whiteKings;
    BitboardElement _blackPawns;
    BitboardElement _blackKnights;
    BitboardElement _blackKings;
    BitboardElement _WhitePieces;
    BitboardElement _BlackPieces;
    BitboardElement _Pieces;

    BitboardElement _knightMoves[64];
    BitboardElement _kingMoves[64];

    Grid* _grid;
};