#include "Chess.h"
#include <limits>
#include <cmath>

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    int tag = static_cast<int>(piece) + (playerNumber == 1 ? 128 : 0);
    bit->setGameTag(tag);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    generateKnightMoveBitboards();
    generateKingMoveBitboards();
    generateAllBitboards();
    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)

    // row and col variables to place pieces
    int row = 0;
    int col = 0;

    // go through each char in fen
    for(char c : fen){
        // check if '/' to move to the next row
        if(c == '/'){
            row++;
            col = 0;
        } 
        // move along columns if its a number
        else if (isdigit(c)){
            col = c - '0';
        }
        // place piece
        else{
            // get square to place piece
            ChessSquare* square = _grid->getSquare(col, row);

            // check if uppercase
            bool isUpperCase = isupper(c);
            int playerNum = isUpperCase ? 1 : 0 ;

            // different pieces
            if (c == 'r' || c == 'R') {
                Bit* piece = PieceForPlayer(playerNum, Rook);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
            else if (c == 'n' || c == 'N') {
                Bit* piece = PieceForPlayer(playerNum,Knight);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
            else if (c == 'b' || c == 'B') {
                Bit* piece = PieceForPlayer(playerNum,Bishop);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
            else if (c == 'q' || c == 'Q') {
                Bit* piece = PieceForPlayer(playerNum,Queen);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
            else if (c == 'k' || c == 'K') {
                Bit* piece = PieceForPlayer(playerNum,King);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
            else if (c == 'p' || c == 'P') {
                Bit* piece = PieceForPlayer(playerNum,Pawn);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
            // go to the next col
            col++;
        }
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    return (bit.gameTag() & 128) == (getCurrentPlayer()->playerNumber() * 128);
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    generateAllBitboards(); 

    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);
    if (!srcSquare || !dstSquare) return false;

    int srcIndex = squareToIndex(srcSquare->getColumn(), srcSquare->getRow());
    int dstIndex = squareToIndex(dstSquare->getColumn(), dstSquare->getRow());

    std::vector<BitMove> moves = generateAllMoves(srcIndex);
    for (const auto& move : moves) {
        if (move.to == dstIndex) {
            return true; 
        }
    }
    return false;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

// BITBOARD STUFF BELOW

// helpers
int Chess::squareToIndex(int x, int y) {
    return y * 8 + x; 
}
void Chess::indexToSquare(int index, int &x, int &y) {
    y = index / 8;
    x = index % 8;
}


void Chess::generateAllBitboards() {
    // clear
    _WhitePieces.setData(0ULL);
    _BlackPieces.setData(0ULL);

    
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit* bit = square->bit();
        if (!bit) return;
        int index = squareToIndex(x, y);
        uint64_t mask = 1ULL << index;
        bool isWhite = (bit->gameTag() < 128);
        ChessPiece piece = static_cast<ChessPiece>(bit->gameTag() % 128);
        if (isWhite) {
            _WhitePieces |= mask;
            switch (piece) { case Pawn: _whitePawns |= mask; break; case Knight: _whiteKnights |= mask; break; case King: _whiteKings |= mask; break; default: break; }
        } else {
            _BlackPieces |= mask;
            switch (piece) { case Pawn: _blackPawns |= mask; break; case Knight: _blackKnights |= mask; break; case King: _blackKings |= mask; break; default: break; }
        }
    });
    _Pieces.setData(_WhitePieces.getData() | _BlackPieces.getData());
}

// knight moves
void Chess::generateKnightMoveBitboards() {
    const int offsets[8][2] = { 
        {2,1}, {2,-1}, 
        {-2,1}, {-2,-1}, 
        {1,2}, {1,-2}, 
        {-1,2}, {-1,-2} 
    }; 
    for(int square = 0; square < 64 ;square++){
        int row = square / 8;
        int col = square % 8;
        uint64_t a = 0; 
        for(auto& offset : offsets){
            int r = row + offset[0];
            int c = col + offset[1]; 
            if(r >= 0 && r < 8 && c >= 0 && c < 8){
                int to = r * 8 + c ; a |= (1ULL << to);
            }
            
        } 
        _knightMoves[square] = BitboardElement(a); 
    } 
}

// king moves
void Chess::generateKingMoveBitboards() {
    int offsets[8][2] = { 
        {0,1},{0,-1},
        {1,0},{-1,0},
        {1,1},{1,-1},
        {-1,1},{-1,-1} 
    };

    for(int square = 0; square < 64 ;square++){
        int row = square / 8;
        int col = square % 8;
        uint64_t a = 0; 
        for(auto& offset : offsets){
            int r = row + offset[0];
            int c = col + offset[1]; 
            if(r >= 0 && r < 8 && c >= 0 && c < 8){
                int to = r * 8 + c ; a |= (1ULL << to);
            }
            
        } 
        _kingMoves[square] = BitboardElement(a); 
    } 
}

// didn't need but keeping just in case
// std::vector<BitMove> Chess::generatePawnMoves(int square, bool isWhite) {
//     std::vector<BitMove> moves;
//     int row = square / 8;
//     int col = square % 8;

//     int direction = isWhite ? 1 : -1;
//     int startRow = isWhite ? 1 : 6;

//     // forward
//     int targetRow = row + direction;
//     if (targetRow >= 0 && targetRow < 8) {
//         int targetSquare = targetRow * 8 + col;
//         if (!(_Pieces.getData() & (1ULL <<  targetSquare))) {
//             moves.emplace_back(square, targetSquare, Pawn);

//             // double
//             if (row == startRow) {
//                 int doubleSquare = (row+2*direction)*8 + col;
//                 if (!(_Pieces.getData() & (1ULL << doubleSquare)))
//                     moves.emplace_back(square, doubleSquare, Pawn);
//             }
//         }
//     }
//     return moves;
// }


std::vector<BitMove> Chess::generateAllMoves(int square) {
    std::vector<BitMove> moves;
    int x, y;
    indexToSquare(square, x, y);
    ChessSquare* sq = _grid->getSquare(x, y);
    if (!sq || !sq->bit()) return moves;

    Bit* bit = sq->bit();
    bool isWhite = (bit->gameTag() < 128);
    ChessPiece piece = static_cast<ChessPiece>(bit->gameTag() % 128);

    uint64_t friendlyPieces = isWhite ? _WhitePieces.getData() : _BlackPieces.getData();
    uint64_t enemyPieces = isWhite ? _BlackPieces.getData() : _WhitePieces.getData();

    switch (piece) {
        case Knight:
        case King: {
            BitboardElement allMoves = (piece == Knight ? _knightMoves[square] : _kingMoves[square]);
            // exclude friendly
            BitboardElement validMoves(allMoves.getData() & ~friendlyPieces);

            validMoves.forEachBit([&](int to) {
                moves.emplace_back(square, to, piece);
            });
            break;
        }

        // actual pawn logic
        case Pawn: {
            int row = square / 8, col = square % 8;
            int direction = isWhite ? 1 : -1;
            int startRank = isWhite ? 1 : 6;

            // move forward
            int targetRank = row + direction;
            if (targetRank >= 0 && targetRank < 8) {
                int targetSquare = targetRank * 8 + col;
                if (!(_Pieces.getData() & (1ULL << targetSquare))) {
                    moves.emplace_back(square, targetSquare, Pawn);

                    // double 
                    if (row == startRank) {
                        int doubleSquare = (row + 2 * direction) * 8 + col;
                        if (!(_Pieces.getData() & (1ULL << doubleSquare)))
                            moves.emplace_back(square, doubleSquare, Pawn);
                    }
                }
            }

            // capturin other pieces
            for (int df : { -1, 1 }) {
                int f = col + df;
                if (f < 0 || f >= 8) continue;
                int captureSquare = targetRank * 8 + f;
                if (enemyPieces & (1ULL << captureSquare)) {
                    moves.emplace_back(square, captureSquare, Pawn);
                }
            }
            break;
            // todo add other pieeces
        }
        default: break;
    }
    return moves;
}


