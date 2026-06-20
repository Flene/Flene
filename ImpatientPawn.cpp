//----------------------------------------------------------------------------------------------------------------------
/// The ImpatientPawn class represents a specialized pawn variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "ImpatientPawn.hpp"
#include "Player.hpp"
#include "Utils.hpp"

ImpatientPawn::ImpatientPawn(PlayerId owner)
  : Pawn(owner)
{
  piece_id_ = "PIPT";
  short_name_ = (owner == PlayerId::WHITE) ? "♟Pi" : "♟pi";
}

bool ImpatientPawn::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Pawn::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string ImpatientPawn::special(Player*, int file, int rank, std::vector<std::string> parameters, Board* board)
{
  std::string promotion_type = parameters.at(1);
  Utils::toUpperCase(promotion_type);
  Pawn::promotion(promotion_type, *board, file, rank);
  return "";
}

std::vector<std::string> ImpatientPawn::canSpecial(Board*)
{
  // Very simple case here. As long as player can afford the mana, this special can be used.
  std::string coordinate_string = coordinates_.toString();
  return 
  {
    "special " + coordinate_string + " Q",
    "special " + coordinate_string + " B",
    "special " + coordinate_string + " N",
    "special " + coordinate_string + " R"
  };
}
