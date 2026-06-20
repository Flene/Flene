//----------------------------------------------------------------------------------------------------------------------
/// The ArcherKing class represents a specialized king variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "ArcherKing.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include "Coordinates.hpp"
#include <cstdlib>

ArcherKing::ArcherKing(PlayerId owner)
  : King(owner)
{
  piece_id_ = "KARC";
  short_name_ = (owner == PlayerId::WHITE) ? "♚Ka" : "♚ka";
}

bool ArcherKing::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  return King::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string ArcherKing::special(Player*, int file, int rank,
                                std::vector<std::string> parameters, Board* board)
{
  Coordinates target_coordinates(parameters.at(1));
  int target_file = target_coordinates.getFile();
  int target_rank = target_coordinates.getRank();

  int direction = (owner_ == PlayerId::WHITE) ? 1 : -1;
  if(target_file != file || (target_rank - rank) * direction <= 0)
  {
    return "E_INVALID_ARCHER_TARGET";
  }

  Square* target_square = board->getSquare(target_file, target_rank);
  Piece* target_piece = target_square ? target_square->getPiece() : nullptr;
  if(target_piece == nullptr || target_piece->getOwner() == owner_)
  {
    return "E_OPPONENT_PIECE_NOT_FOUND";
  }
  target_piece->setFrozenTurns(1);
  return "";
}

std::vector<std::string> ArcherKing::canSpecial(Board* board)
{
  std::vector<std::string> specials;
  int file = coordinates_.getFile();
  int rank = coordinates_.getRank();
  int direction = (owner_ == PlayerId::WHITE) ? 1 : -1;
  for (int r = rank + direction; r >= 0 && r < 8; r += direction)
  {
    Square* sq = board->getSquare(file, r);
    if (!sq) break;
    Piece* p = sq->getPiece();
    if (!p) continue;
    if (p->getOwner() == owner_)break;

    std::string from = std::string(1, 'a' + file) + std::to_string(rank + 1);
    std::string to = std::string(1, 'a' + file) + std::to_string(r + 1);
    specials.push_back("special " + from + " " + to);
    break;
  }
  return specials;
}
