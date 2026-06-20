//----------------------------------------------------------------------------------------------------------------------
/// The Pawn class represents a standard pawn piece, implementing specific movement logic, 
/// capture rules, en passant logic, and promotion handling.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Pawn.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include <cstdlib>

Pawn::Pawn(PlayerId owner)
    : Piece(PieceType::PAWN, owner, "P", (owner == PlayerId::WHITE ? "♟P" : "♟p"), 1) 
{}

bool Pawn::move(int start_rank, int start_file, int target_row, int target_col, 
                bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  int direction = (owner_ == PlayerId::WHITE) ? 1 : -1;
  int initial_row = (owner_ == PlayerId::WHITE) ? 1 : 6;

  int row_diff = target_row - start_rank;
  int col_diff = std::abs(target_col - start_file);

  Square* dest_square = board.getSquare(target_col, target_row);
  if(dest_square == nullptr) return false;
  Piece* target_piece = dest_square->getPiece();

  bool en_passant = false;

  if(is_capture) 
  {
    if(row_diff != direction || col_diff != 1) return false;
    if(target_piece == nullptr)
    {
      en_passant = board.isEnPassantCapture(owner_, start_rank, start_file, target_row, target_col);
      if(!en_passant) return false;
    }
    else if(target_piece->getOwner() == owner_)
    {
      return false;
    }
  } 
  else 
  {
    if(col_diff != 0) return false;
    if(target_piece != nullptr) return false;

    if(row_diff == direction) 
    {
      // one step forward allowed
    } 
    else if(row_diff == 2 * direction && start_rank == initial_row && !hasMoved()) 
    {
      Square* intermediate_square = board.getSquare(start_file, start_rank + direction);
      if(intermediate_square == nullptr || intermediate_square->getPiece() != nullptr) 
      {
        return false;
      }
    } 
    else return false;
  }

  if(en_passant)
  {
    Square* source_square = board.getSquare(start_file, start_rank);
    Square* captured_square = board.getEnPassantCapturedSquare(owner_, start_rank, start_file, target_row, target_col);
    Piece* captured_piece = captured_square ? captured_square->getPiece() : nullptr;
    if(source_square == nullptr || source_square->getPiece() != this || captured_piece == nullptr) return false;
    if(getFrozen() || isInvincible()) return false;
    if(captured_piece->isInvincible()) return false;
    if(dry_run) return true;

    if(captured_piece->hasItem("SHIELD"))
    {
      captured_piece->clearItem();
      return true;
    }

    if(captured_piece->hasItem())
    {
      setItem(captured_piece->getItemId());
      captured_piece->clearItem();
    }
    std::unique_ptr<Piece> captured = captured_square->releasePiece();
    active_player.addPieceToPrison(std::move(captured));

    std::unique_ptr<Piece> moving_piece = source_square->releasePiece();
    if(moving_piece)
    {
      moving_piece->setCoordinates(Coordinates(target_col, target_row));
      moving_piece->setHasMoved(true);
    }
    dest_square->setPiece(std::move(moving_piece));
    Piece* moved_piece = dest_square->getPiece();
    if(moved_piece != nullptr)
    {
      dest_square->executeEffect(active_player, *moved_piece);
    }
    return true;
  }

  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
}

void Pawn::promotion(const std::string& type, Board& board, int file, int rank)
{
  Square* square = board.getSquare(file, rank);
  if(square == nullptr) return;
  Piece* old_piece = square->getPiece();
  PlayerId owner = old_piece ? old_piece->getOwner() : owner_;

  std::unique_ptr<Piece> new_piece;
  if(type == "R")       new_piece = std::make_unique<Rook>(owner);
  else if(type == "N")  new_piece = std::make_unique<Knight>(owner);
  else if(type == "B")  new_piece = std::make_unique<Bishop>(owner);
  else if(type == "Q")  new_piece = std::make_unique<Queen>(owner);

  if(new_piece)
  {
    new_piece->setCoordinates(Coordinates(file, rank));
    new_piece->setHasMoved(true);
    new_piece->initializeOnSquare(square);
    square->setPiece(std::move(new_piece));
  }
}
