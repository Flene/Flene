//----------------------------------------------------------------------------------------------------------------------
/// The Knight class represents a standard knight piece and implements the specific movement logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Knight.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include <cstdlib>

Knight::Knight(PlayerId owner)
    : Piece(PieceType::KNIGHT, owner, "N", (owner == PlayerId::WHITE ? "♞N" : "♞n"), 3)
{}

bool Knight::move(int start_rank, int start_file, int target_row, int target_col, 
                bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  int row_diff = std::abs(target_row - start_rank);
  int col_diff = std::abs(target_col - start_file);
  if(!((row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2))) return false;
  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
}
