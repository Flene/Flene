//----------------------------------------------------------------------------------------------------------------------
/// The FrightenedKing class represents a specialized king variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "FrightenedKing.hpp"
#include "Player.hpp"

FrightenedKing::FrightenedKing(PlayerId owner)
  : King(owner)
{
  piece_id_ = "KFRT";
  short_name_ = (owner == PlayerId::WHITE) ? "♚Kf" : "♚kf";
}

bool FrightenedKing::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  return King::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}
