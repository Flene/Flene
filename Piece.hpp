//----------------------------------------------------------------------------------------------------------------------
/// The Piece class serves as the base class for all game pieces, managing core attributes like 
/// ownership, positions, movement paths, status flags, and item interactions.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#ifndef PIECE_HPP
#define PIECE_HPP

#include <string>
#include "Coordinates.hpp"
#include <vector>
#include <memory>

enum class PlayerId;
class Board;
class Player;
class Square;

enum class PieceType
{
  PAWN,
  ROOK,
  KNIGHT,
  BISHOP,
  QUEEN,
  KING
};

class Piece
{
  protected:
    PieceType piece_type_;
    std::string piece_id_;
    std::string short_name_;
    int value_;
    PlayerId owner_;
    Coordinates coordinates_;
    bool is_captured_;
    bool is_frozen_ = false;
    int frozen_turns_ = 0;
    int invincible_turns_ = 0;
    std::string item_id_;
    int even_odd_restriction_ = 0; // 0 = none, 1 = odd turns only, 2 = even turns only
    bool has_moved_ = false;

    bool isPathClear(int start_rank, int start_file, int target_row, int target_col, Board& board) const;
    bool movePieceOnBoard(int start_rank, int start_file, int target_row, int target_col,
                          bool is_capture, Board& board, Player& active_player,
                          bool allow_friendly_capture = false, bool dry_run = false);

  public:
    Piece(PieceType type, PlayerId owner, const std::string& id, const std::string& short_name, int value);

    virtual ~Piece() = default;

    virtual bool move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run = false) = 0;

    // Clone the piece (deep): returns a new heap-allocated Piece instance with same runtime type and state.
    virtual std::unique_ptr<Piece> clone() const;

    // Required by derived pieces
    virtual PieceType getType() const { return piece_type_; }
    virtual std::string getPieceId() const { return piece_id_; }
    virtual std::string getShortName() const { return short_name_; }
    int getValue() const { return value_; }
    PlayerId getOwner() const { return owner_; }
    bool isInvincible() const { return invincible_turns_ > 0; }
    int getFrozenTurns() const { return frozen_turns_; }
    int getInvincibleTurns() const { return invincible_turns_; }
    Coordinates getCoordinates() const { return coordinates_; }
    bool getCaptured() const { return is_captured_; }
    bool getFrozen() const { return is_frozen_ || frozen_turns_ > 0; }

    bool hasItem() const { return !item_id_.empty(); }
    bool hasItem(const std::string& item_id) const { return item_id_ == item_id; }
    const std::string& getItemId() const { return item_id_; }
    std::string getItemDisplayName() const;
    void setItem(const std::string& item_id);
    void clearItem();

    bool canMoveOnTurn(int turn_count) const;
    bool hasMoved() const { return has_moved_; }
    void setHasMoved(bool moved) { has_moved_ = moved; }
    void setEvenOddRestriction(const std::string& parity);
    virtual std::string special(Player*, int, int, std::vector<std::string>, Board*) { return ""; };
    virtual bool hasSpecial() { return false; }; // A3
    virtual std::vector<std::string> canSpecial(Board*) { return {""}; };
    // source_square, (piece_type arbitrary), target_square, (turn_count arbitrary), bounce_square
    virtual void initializeOnSquare(Square*) {}

    void setCoordinates(const Coordinates& coordinates);
    void setCaptured(const bool is_captured);
    void setFrozen(const bool is_frozen);
    void setFrozenTurns(int turns);
    void setInvincibleTurns(int turns);
    void decreaseStatusTimers();
    void setOwner(PlayerId owner);
};
#endif