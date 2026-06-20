//----------------------------------------------------------------------------------------------------------------------
/// The Board class represents the chess board, managing grid coordinates, square types, 
/// piece placement, item spawning, en passant rules and board printing.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#ifndef BOARD_HPP
#define BOARD_HPP
#define BG_DARK    "\033[48;5;94m"
#define BG_LIGHT   "\033[48;5;223m"
#define BG_MANA    "\033[48;5;32m" // blue
#define BG_BOOST   "\033[48;5;226m" // yelllow
#define BG_SPAWN   "\033[48;5;28m" // green
#define FG_WHITE   "\033[1;38;5;247m"
#define FG_BLACK   "\033[1;38;5;16m"
#define RESET      "\033[0m"

#include <memory>
#include <array>
#include <vector>
#include "Coordinates.hpp"
#include "Square.hpp"
#include "Player.hpp"
#include "King.hpp"

class Piece;

class Board
{
private:
    bool active_ = true;
    std::unique_ptr<Square> grid_[8][8];
    bool en_passant_available_ = false;
    int en_passant_file_ = -1;
    int en_passant_rank_ = -1;
    PlayerId en_passant_owner_;

public:
    Board();
    Board(const Board& other) = delete;
    ~Board();

    // Print the board from the given player's perspective.
    void print(PlayerId perspective, int turn, int max_turns, int white_mana, int black_mana, int max_mana, bool can_pass = false);

    // Place a Piece at file (column), rank (row).
    void placePiece(int file, int rank, std::unique_ptr<Piece> piece);

    // Replace the Square at file, rank with a special Square.
    void placeSquare(int file, int rank, std::unique_ptr<Square> new_square);

    // Returns true if the board is active (could be used for toggling boards or variants).
    bool getActive_() const;

    // Set if the board is active.
    void setActive_(bool active);

    // Get a non-owning pointer to the Square at file, rank.
    Square* getSquare(int file, int rank);
    const Square* getSquare(int file, int rank) const;
    King* getKing(PlayerId owner);
    void decreaseStatusTimers(PlayerId owner);
    int countPiecesOnManaSquares(PlayerId owner) const;
    void spawnItemsForRound(int round_number);

    void setEnPassantPawn(int file, int rank, PlayerId owner);
    void clearEnPassant();
    bool isEnPassantCapture(PlayerId capturer, int start_rank, int start_file,
                            int target_rank, int target_file) const;
    Square* getEnPassantCapturedSquare(PlayerId capturer, int start_rank, int start_file,
                                       int target_rank, int target_file);

    // Snapshot types for undoing arbitrary commands (Stage 2)
    struct SquareSnapshot
    {
      std::unique_ptr<Piece> piece; // cloned piece (if any)
      std::string type;
      SquareColor color;
      std::string item_id;
    };

    struct UndoRecord
    {
      bool valid = false;

      // en-passant snapshot
      bool en_passant_prev_available = false;
      int en_passant_prev_file = -1;
      int en_passant_prev_rank = -1;
      PlayerId en_passant_prev_owner = PlayerId::WHITE;

      // mana snapshot
      int white_mana_before = 0;
      int black_mana_before = 0;

      // full-board snapshot
      std::array<std::array<SquareSnapshot,8>,8> squares;

      // prison snapshots
      std::vector<std::unique_ptr<Piece>> white_prison_before;
      std::vector<std::unique_ptr<Piece>> black_prison_before;
    };

    
    
    UndoRecord makeMoveSimulation(const std::string& command_string, Player& active_player, Player& opponent,
                                  int turn_count, bool frightened_king_cannot_capture);


    void undoMoveSimulation(UndoRecord& rec, Player& active_player, Player& opponent);
};

#endif