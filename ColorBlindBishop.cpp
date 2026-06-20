//----------------------------------------------------------------------------------------------------------------------
/// The ColorBlindBishop class represents a specialized bishop variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "ColorBlindBishop.hpp"
#include "Player.hpp"

ColorBlindBishop::ColorBlindBishop(PlayerId owner)
  : Bishop(owner)
{
  piece_id_ = "BCLR";
  short_name_ = (owner == PlayerId::WHITE) ? "♝Bc" : "♝bc";
}

bool ColorBlindBishop::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Bishop::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}


std::string ColorBlindBishop::special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board)
{
  Coordinates target_coords(parameters.at(1));
  int target_file = target_coords.getFile();
  int target_rank = target_coords.getRank();

  int file_diff = std::abs(target_file - file);
  int rank_diff = std::abs(target_rank - rank);

  if(file_diff > 1 || rank_diff > 1 || (file_diff == 0 && rank_diff == 0)) return "E_INVALID_MOVE";

  Square* target_square = board->getSquare(target_file, target_rank);
  if(!target_square) return "E_INVALID_MOVE";

  Piece* target_piece = target_square->getPiece();
  bool is_capture = (target_piece != nullptr && target_piece->getOwner() != owner_);

  if(target_piece != nullptr && target_piece->getOwner() == owner_) return "E_INVALID_MOVE";

  if(!movePieceOnBoard(rank, file, target_rank, target_file, is_capture, *board, *active_player, false, false))
  {
    return "E_INVALID_MOVE";
  }
  if(target_square->getType() == "NORMAL")
  {
    setInherentColor(target_square->getColor());
  }
  return "";
}

std::vector<std::string> ColorBlindBishop::canSpecial(Board* board)
{ 
  std::vector<std::string> specials;
  Coordinates current_coords = getCoordinates();
  int file = current_coords.getFile();
  int rank = current_coords.getRank();
  for(int dr = -1; dr <= 1; ++dr)
  {
    for(int df = -1; df <= 1; ++df)
    {
      if(dr == 0 && df == 0) continue;
      int target_rank = rank + dr;
      int target_file = file + df;
      Square* target_square = board->getSquare(target_file, target_rank);
      if(target_square)
      {
        Piece* target_piece = target_square->getPiece();
        if(!target_piece || target_piece->getOwner() != owner_)
        {
          std::string special_command = "special " + std::string(1, 'a' + file) + 
          std::to_string(rank + 1) + " " + std::string(1, 'a' + target_file) + 
          std::to_string(target_rank + 1);
          specials.push_back(special_command);
        }
      }
    }
  }
  return specials;
}