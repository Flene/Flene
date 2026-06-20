//----------------------------------------------------------------------------------------------------------------------
/// The PreacherBishop class represents a specialized bishop variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "PreacherBishop.hpp"
#include "Player.hpp"

PreacherBishop::PreacherBishop(PlayerId owner)
  : Bishop(owner)
{
  piece_id_ = "BPRC";
  short_name_ = (owner == PlayerId::WHITE) ? "♝Bp" : "♝bp";
}

bool PreacherBishop::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Bishop::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string PreacherBishop::special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board)
{
  Coordinates target_coordinates = Coordinates(parameters.at(1));
  int target_file = target_coordinates.getFile();
  int target_rank = target_coordinates.getRank();
  Square* target_square = board->getSquare(target_file, target_rank);

  bool valid_move = Bishop::move(rank, file, target_rank, target_file, true, *board, *active_player, true);
  if(!valid_move) return "E_INVALID_MOVE";

  Piece* target_piece = target_square->getPiece();
  PreacherBishop* preacher_bishop = dynamic_cast<PreacherBishop*>(target_piece);
  if(preacher_bishop != nullptr) return "E_UNWAVERING_FAITH";
  target_piece->setOwner(active_player->getId());
  return "";
}

std::vector<std::string> PreacherBishop::canSpecial(Board* board)
{
  std::vector<std::string> specials;
  int current_file = coordinates_.getFile();
  int current_rank = coordinates_.getRank();
  std::vector<std::pair<int,int>> directions = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
  for (auto [df, dr] : directions)
  {
    int tf = current_file + df;
    int tr = current_rank + dr;
    while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8)
    {
      Square* sq = board->getSquare(tf, tr);
      if (!sq) break;

      Piece* p = sq->getPiece();
      if (p && p->getOwner() == owner_) break;
      if (p && dynamic_cast<PreacherBishop*>(p) != nullptr) break;
      if (p && p->getOwner() != owner_)
      {
        std::string from = std::string(1, 'a' + current_file) + std::to_string(current_rank + 1);
        std::string to = std::string(1, 'a' + tf) + std::to_string(tr + 1);
        specials.push_back("special " + from + " " + to);
        break;
      }
      tf += df;
      tr += dr;
    }
  }
  return specials;
}

