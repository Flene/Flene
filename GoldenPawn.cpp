//----------------------------------------------------------------------------------------------------------------------
/// The GoldenPawn class represents a specialized pawn variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "GoldenPawn.hpp"
#include "Player.hpp"

GoldenPawn::GoldenPawn(PlayerId owner)
  : Pawn(owner)
{
  piece_id_ = "PGLD";
  short_name_ = (owner == PlayerId::WHITE) ? "♟Pg" : "♟pg";
}

bool GoldenPawn::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  return Pawn::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

void GoldenPawn::promotion(const std::string&, Board&, int, int)
{
  // Golden Pawn ends game.
}
