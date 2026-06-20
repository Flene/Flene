//----------------------------------------------------------------------------------------------------------------------
/// The Queen class represents a standard queen piece and implements the specific movement logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Queen.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include <cstdlib>

Queen::Queen(PlayerId owner)
    : Piece(PieceType::QUEEN, owner, "Q", (owner == PlayerId::WHITE ? "♛Q" : "♛q"), 9)
{}

bool Queen::move(int start_rank, int start_file, int target_row, int target_col, 
                bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  int row_diff = std::abs(target_row - start_rank);
  int col_diff = std::abs(target_col - start_file);
  bool straight = (start_rank == target_row || start_file == target_col);
  bool diagonal = (row_diff == col_diff);
  if(start_rank == target_row && start_file == target_col) return false;
  if(!straight && !diagonal) return false;
  if(!isPathClear(start_rank, start_file, target_row, target_col, board)) return false;
  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, allow_friendly_capture_, dry_run);
}
