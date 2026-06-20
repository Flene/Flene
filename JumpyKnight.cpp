//----------------------------------------------------------------------------------------------------------------------
/// The JumpyKnight class represents a specialized knight variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "JumpyKnight.hpp"
#include "Player.hpp"

JumpyKnight::JumpyKnight(PlayerId owner)
  : Knight(owner)
{
  piece_id_ = "NJMP";
  short_name_ = (owner == PlayerId::WHITE) ? "♞Nj" : "♞nj";
}

bool JumpyKnight::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  int row_diff = std::abs(target_row - start_rank);
  int col_diff = std::abs(target_col - start_file);
  if(!((row_diff == 3 && col_diff == 2) || (row_diff == 2 && col_diff == 3))) return false;
  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
}
