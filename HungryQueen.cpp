//----------------------------------------------------------------------------------------------------------------------
/// The HungryQueen class represents a specialized queen variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "HungryQueen.hpp"
#include "Player.hpp"

HungryQueen::HungryQueen(PlayerId owner)
  : Queen(owner)
{
  piece_id_ = "QHNGR";
  short_name_ = (owner == PlayerId::WHITE) ? "♛Qh" : "♛qh";
  allow_friendly_capture_ = true;
}

bool HungryQueen::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  return Queen::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}
