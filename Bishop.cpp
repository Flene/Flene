//----------------------------------------------------------------------------------------------------------------------
/// The Bishop class represents a standard bishop piece and implements the specific movement logic, 
/// restricted by its color-bound square rules.
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Bishop.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include <cstdlib>

Bishop::Bishop(PlayerId owner)
    : Piece(PieceType::BISHOP, owner, "B", (owner == PlayerId::WHITE ? "♝B" : "♝b"), 3),
      inherent_color_(owner == PlayerId::WHITE ? SquareColor::WHITE : SquareColor::BLACK)
{}

void Bishop::initializeOnSquare(Square* square)
{
  if(square == nullptr) return;

  if(square->getType() == "NORMAL")
  {
    inherent_color_ = square->getColor();
  }
  else
  {
    inherent_color_ = (owner_ == PlayerId::WHITE) ? SquareColor::WHITE : SquareColor::BLACK;
  }
}

bool Bishop::move(int start_rank, int start_file, int target_row, int target_col,
                  bool is_capture, Board& board, Player& active_player, bool dry_run)
{
  int diff_file = target_col - start_file;
  int diff_rank = target_row - start_rank;

  if (!(diff_file == 0 || diff_rank == 0 || std::abs(diff_file) == std::abs(diff_rank))) return false;
  if (diff_file == 0 && diff_rank == 0) return false;

  if (!isPathClear(start_rank, start_file, target_row, target_col, board)) return false;

  SquareColor inherent = getInherentColor();
  int step_file = (diff_file > 0) ? 1 : (diff_file < 0 ? -1 : 0);
  int step_rank = (diff_rank > 0) ? 1 : (diff_rank < 0 ? -1 : 0);

  int f = start_file + step_file;
  int r = start_rank + step_rank;
  while (f != target_col || r != target_row)
  {
    Square* sq = board.getSquare(f, r);
    if (sq)
    {
      if (sq->getType() == "NORMAL" && sq->getColor() != inherent) return false;
    }
    f += step_file;
    r += step_rank;
  }

  Square* target_sq = board.getSquare(target_col, target_row);
  if (target_sq)
  {
    if (target_sq->getType() == "NORMAL" && target_sq->getColor() != inherent) return false;
  }
  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
}