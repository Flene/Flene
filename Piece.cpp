//----------------------------------------------------------------------------------------------------------------------
/// The Piece class serves as the base class for all game pieces, managing core attributes like 
/// ownership, positions, movement logic, status flags, and item interactions.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#include "Piece.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include "Player.hpp"

#include <algorithm>

// include concrete piece headers so we can construct clones
#include "Pawn.hpp"
#include "GoldenPawn.hpp"
#include "ImpatientPawn.hpp"
#include "StubbornPawn.hpp"
#include "NervousPawn.hpp"
#include "ExplosivePawn.hpp"
#include "Rook.hpp"
#include "InvincibleRook.hpp"
#include "PainterRook.hpp"
#include "Knight.hpp"
#include "JumpyKnight.hpp"
#include "IceKnight.hpp"
#include "Bishop.hpp"
#include "ColorBlindBishop.hpp"
#include "PreacherBishop.hpp"
#include "Queen.hpp"
#include "FlipperQueen.hpp"
#include "JumpyQueen.hpp"
#include "HungryQueen.hpp"
#include "King.hpp"
#include "FrightenedKing.hpp"
#include "ArcherKing.hpp"

#include <memory>

Piece::Piece(PieceType type, PlayerId owner, const std::string& id, const std::string& short_name, int value)
    : piece_type_(type), piece_id_(id), short_name_(short_name), value_(value), owner_(owner), coordinates_(0, 0),
      is_captured_(false), is_frozen_(false), frozen_turns_(0), invincible_turns_(0), item_id_(""), even_odd_restriction_(0), has_moved_(false)
{}

bool Piece::isPathClear(int start_rank, int start_file, int target_row, int target_col, Board& board) const
{
  int row_step = (target_row > start_rank) ? 1 : (target_row < start_rank ? -1 : 0);
  int col_step = (target_col > start_file) ? 1 : (target_col < start_file ? -1 : 0);

  int row = start_rank + row_step;
  int col = start_file + col_step;
  while(row != target_row || col != target_col)
  {
    Square* square = board.getSquare(col, row);
    if(square == nullptr || square->getPiece() != nullptr)
    {
      return false;
    }
    row += row_step;
    col += col_step;
  }
  return true;
}

bool Piece::movePieceOnBoard(int start_rank, int start_file, int target_row, int target_col,
                             bool is_capture, Board& board, Player& active_player,
                             bool allow_friendly_capture, bool dry_run)
{
  if(getFrozen() || isInvincible()) return false;

  Square* source_square = board.getSquare(start_file, start_rank);
  Square* target_square = board.getSquare(target_col, target_row);
  if(source_square == nullptr || target_square == nullptr) return false;
  if(source_square->getPiece() != this) return false;

  Piece* target_piece = target_square->getPiece();
  if(is_capture)
  {
    if(target_piece == nullptr) return false;
    if(target_piece->getOwner() == owner_)
    {
      if(!allow_friendly_capture) return false;
      if(target_piece->getType() == PieceType::KING) return false;
    }
  }
  else
  {
    if(target_piece != nullptr) return false;
  }

  if(target_piece != nullptr)
  {
    if(target_piece->isInvincible()) return false;
    if(piece_type_ == PieceType::QUEEN && target_piece->hasItem("REPEL")) return false;

    if(dry_run) return true;

    if(target_piece->hasItem("SHIELD"))
    {
      target_piece->clearItem();
      return true;
    }

    if(target_piece->hasItem())
    {
      setItem(target_piece->getItemId());
      target_piece->clearItem();
    }

    std::unique_ptr<Piece> captured_piece = target_square->releasePiece();
    active_player.addPieceToPrison(std::move(captured_piece));
  }
  if(dry_run) return true;

  std::unique_ptr<Piece> moving_piece = source_square->releasePiece();
  if(moving_piece)
  {
    moving_piece->setCoordinates(Coordinates(target_col, target_row));
    moving_piece->setHasMoved(true);
  }
  target_square->setPiece(std::move(moving_piece));
  Piece* moved_piece = target_square->getPiece();
  if(moved_piece != nullptr)
  {
    if(target_square->getType() != "NORMAL")
    {
      target_square->executeEffect(active_player, *moved_piece);
    }
  }
  return true;
}

std::string Piece::getItemDisplayName() const
{
  if(item_id_ == "FREEZE") return "*";
  if(item_id_ == "TP") return "→";
  if(item_id_ == "EVENODD") return "½";
  if(item_id_ == "LUKE") return "↑";
  if(item_id_ == "SHIELD") return "□";
  if(item_id_ == "CLOAK") return "⌂";
  if(item_id_ == "REPEL") return "R";
  return " ";
}

void Piece::setItem(const std::string& item_id)
{
  item_id_ = item_id;
}

void Piece::clearItem()
{
  item_id_.clear();
}

bool Piece::canMoveOnTurn(int turn_count) const
{
  if(even_odd_restriction_ == 0) return true;
  bool even_turn = (turn_count % 2) == 0;
  if(even_odd_restriction_ == 2) return even_turn;
  return !even_turn;
}

void Piece::setEvenOddRestriction(const std::string& parity)
{
  if(parity == "even") even_odd_restriction_ = 2;
  else if(parity == "odd") even_odd_restriction_ = 1;
  else even_odd_restriction_ = 0;
}

void Piece::setCoordinates(const Coordinates& coordinates)
{ 
  coordinates_ = coordinates;
}
void Piece::setCaptured(const bool is_captured)
{
  is_captured_ = is_captured;
}
void Piece::setFrozen(const bool is_frozen)
{
  is_frozen_ = is_frozen;
  if(!is_frozen) frozen_turns_ = 0;
}

void Piece::setFrozenTurns(int turns)
{
  frozen_turns_ = std::max(0, turns);
  is_frozen_ = frozen_turns_ > 0;
}

void Piece::setInvincibleTurns(int turns)
{
  invincible_turns_ = std::max(0, turns);
}

void Piece::decreaseStatusTimers()
{
  if(frozen_turns_ > 0) --frozen_turns_;
  if(frozen_turns_ == 0) is_frozen_ = false;
  if(invincible_turns_ > 0) --invincible_turns_;
}

void Piece::setOwner(PlayerId owner)
{
  owner_ = owner;
  const bool white = owner_ == PlayerId::WHITE;
  if(piece_id_ == "P") short_name_ = white ? "♟P" : "♟p";
  else if(piece_id_ == "PGLD") short_name_ = white ? "♟Pg" : "♟pg";
  else if(piece_id_ == "PIPT") short_name_ = white ? "♟Pi" : "♟pi";
  else if(piece_id_ == "PSTB") short_name_ = white ? "♟P+" : "♟p+";
  else if(piece_id_ == "PNRV") short_name_ = white ? "♟P-" : "♟p-";
  else if(piece_id_ == "PEXP") short_name_ = white ? "♟P!" : "♟p!";
  else if(piece_id_ == "R") short_name_ = white ? "♜R" : "♜r";
  else if(piece_id_ == "RINV") short_name_ = white ? "♜Ri" : "♜ri";
  else if(piece_id_ == "RPNT") short_name_ = white ? "♜Rp" : "♜rp";
  else if(piece_id_ == "N") short_name_ = white ? "♞N" : "♞n";
  else if(piece_id_ == "NJMP") short_name_ = white ? "♞Nj" : "♞nj";
  else if(piece_id_ == "NICE") short_name_ = white ? "♞Ni" : "♞ni";
  else if(piece_id_ == "B") short_name_ = white ? "♝B" : "♝b";
  else if(piece_id_ == "BCLR") short_name_ = white ? "♝Bc" : "♝bc";
  else if(piece_id_ == "BPRC") short_name_ = white ? "♝Bp" : "♝bp";
  else if(piece_id_ == "Q") short_name_ = white ? "♛Q" : "♛q";
  else if(piece_id_ == "QFLP") short_name_ = white ? "♛Qf" : "♛qf";
  else if(piece_id_ == "QJMP") short_name_ = white ? "♛Qj" : "♛qj";
  else if(piece_id_ == "QHNGR") short_name_ = white ? "♛Qh" : "♛qh";
  else if(piece_id_ == "K") short_name_ = white ? "♚K" : "♚k";
  else if(piece_id_ == "KFRT") short_name_ = white ? "♚Kf" : "♚kf";
  else if(piece_id_ == "KARC") short_name_ = white ? "♚Ka" : "♚ka";
}

// Create a deep copy (clone) of this Piece instance. 
std::unique_ptr<Piece> Piece::clone() const
{
  std::unique_ptr<Piece> copy;

  // Create derived piece instance based on piece_id_
  if (piece_id_ == "P")          copy = std::make_unique<Pawn>(owner_);
  else if (piece_id_ == "PGLD")  copy = std::make_unique<GoldenPawn>(owner_);
  else if (piece_id_ == "PIPT")  copy = std::make_unique<ImpatientPawn>(owner_);
  else if (piece_id_ == "PSTB")  copy = std::make_unique<StubbornPawn>(owner_);
  else if (piece_id_ == "PNRV")  copy = std::make_unique<NervousPawn>(owner_);
  else if (piece_id_ == "PEXP")  copy = std::make_unique<ExplosivePawn>(owner_);

  else if (piece_id_ == "R")     copy = std::make_unique<Rook>(owner_);
  else if (piece_id_ == "RINV")  copy = std::make_unique<InvincibleRook>(owner_);
  else if (piece_id_ == "RPNT")  copy = std::make_unique<PainterRook>(owner_);

  else if (piece_id_ == "N")     copy = std::make_unique<Knight>(owner_);
  else if (piece_id_ == "NJMP")  copy = std::make_unique<JumpyKnight>(owner_);
  else if (piece_id_ == "NICE")  copy = std::make_unique<IceKnight>(owner_);

  else if (piece_id_ == "B")     copy = std::make_unique<Bishop>(owner_);
  else if (piece_id_ == "BCLR")  copy = std::make_unique<ColorBlindBishop>(owner_);
  else if (piece_id_ == "BPRC")  copy = std::make_unique<PreacherBishop>(owner_);

  else if (piece_id_ == "Q")     copy = std::make_unique<Queen>(owner_);
  else if (piece_id_ == "QFLP")  copy = std::make_unique<FlipperQueen>(owner_);
  else if (piece_id_ == "QJMP")  copy = std::make_unique<JumpyQueen>(owner_);
  else if (piece_id_ == "QHNGR") copy = std::make_unique<HungryQueen>(owner_);

  else if (piece_id_ == "K")     copy = std::make_unique<King>(owner_);
  else if (piece_id_ == "KFRT")  copy = std::make_unique<FrightenedKing>(owner_);
  else if (piece_id_ == "KARC")  copy = std::make_unique<ArcherKing>(owner_);

  // If unknown, attempt to create a basic Piece via type (fallback not expected)
  if (!copy)
  {
    // fallback: create a basic pawn as placeholder
    copy = std::make_unique<Pawn>(owner_);
  }

  // Copy protected/internal state
  copy->piece_type_ = piece_type_;
  copy->piece_id_ = piece_id_;
  copy->short_name_ = short_name_;
  copy->value_ = value_;
  copy->owner_ = owner_;
  copy->coordinates_ = coordinates_;
  copy->is_captured_ = is_captured_;
  copy->is_frozen_ = is_frozen_;
  copy->frozen_turns_ = frozen_turns_;
  copy->invincible_turns_ = invincible_turns_;
  copy->item_id_ = item_id_;
  copy->even_odd_restriction_ = even_odd_restriction_;
  copy->has_moved_ = has_moved_;

  return copy;
}