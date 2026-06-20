//----------------------------------------------------------------------------------------------------------------------
/// The StubbornPawn class represents a specialized pawn variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "StubbornPawn.hpp"
#include "Player.hpp"

StubbornPawn::StubbornPawn(PlayerId owner)
  : Pawn(owner)
{
  piece_id_ = "PSTB";
  short_name_ = (owner == PlayerId::WHITE) ? "♟P+" : "♟p+";
}

bool StubbornPawn::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Pawn::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string StubbornPawn::special(Player* active_player, int file, int rank, std::vector<std::string>, Board* board)
{
  int forward = (owner_ == PlayerId::WHITE) ? 1 : -1;
  int target_rank = rank + forward;
  bool return_value = movePieceOnBoard(rank, file, target_rank, file, true, *board, *active_player);
  if(!return_value) return "E_INVALID_MOVE";
  return "";
}

std::vector<std::string> StubbornPawn::canSpecial(Board* board)
{
  std::vector<std::string> specials;
  int forward = (owner_ == PlayerId::WHITE) ? 1 : -1;
  int target_rank = coordinates_.getRank() + forward;
  Square* front = board->getSquare(coordinates_.getFile(), coordinates_.getRank() + forward);
  if (target_rank >= 0 && target_rank < 8 && !front->isEmpty())
  {
    specials.push_back("special " + coordinates_.toString());
  }
  return specials;
}
