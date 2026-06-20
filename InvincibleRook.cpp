//----------------------------------------------------------------------------------------------------------------------
/// The InvincibleRook class represents a specialized rook variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "InvincibleRook.hpp"
#include "Player.hpp"

InvincibleRook::InvincibleRook(PlayerId owner)
  : Rook(owner)
{
  piece_id_ = "RINV";
  short_name_ = (owner == PlayerId::WHITE) ? "♜Ri" : "♜ri";
}

bool InvincibleRook::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Rook::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string InvincibleRook::special(Player*, int, int, std::vector<std::string> parameters, Board*)
{
  int turn_count = std::stoi(parameters.at(1)) + 1;
  setInvincibleTurns(turn_count);
  return "";
}

std::vector<std::string> InvincibleRook::canSpecial(Board*)
{
  std::vector<std::string> specials;
  int current_file = coordinates_.getFile();
  int current_rank = coordinates_.getRank();

  specials.push_back(
      "special " +
      std::string(1, 'a' + current_file) +
      std::to_string(current_rank + 1) +
      " " +
      std::to_string(3) // duration is hardcoded for now
  );

  return specials;
}

