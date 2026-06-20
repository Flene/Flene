//----------------------------------------------------------------------------------------------------------------------
/// The PainterRook class represents a specialized rook variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "PainterRook.hpp"
#include "Player.hpp"

PainterRook::PainterRook(PlayerId owner)
  : Rook(owner)
{
  piece_id_ = "RPNT";
  short_name_ = (owner == PlayerId::WHITE) ? "♜Rp" : "♜rp";
}

bool PainterRook::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Rook::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string PainterRook::special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board)
{
  
  Coordinates target_coordinates = Coordinates(parameters.at(1));
  int target_file = target_coordinates.getFile();
  int target_rank = target_coordinates.getRank();
  Square* source_square = board->getSquare(file, rank);
  Square* target_square = board->getSquare(target_file, target_rank);
  bool is_capture = target_square && target_square->getPiece() != nullptr && target_square->getPiece()->getOwner() != active_player->getId();

  bool return_value = Rook::move(rank, file, target_rank, target_file, is_capture, *board, *active_player);
  if (!return_value) return "E_INVALID_MOVE";
  
  std::string type = source_square->getType();;
  SquareColor square_color;

  if(type == "SPAWN" || type == "BOOST" || type == "MANA")
  {
    square_color = (active_player->getId() == PlayerId::WHITE) ? SquareColor::WHITE : SquareColor::BLACK;
  }
  else
  {
    square_color = source_square->getColor();
  }
  int step_file = 0;
  int step_rank = 0;
  if(target_file != file)
  {
    step_file = (target_file > file) ? 1 : -1;
  }
  if(target_rank != rank)
  {
    step_rank = (target_rank > rank) ? 1 : -1;
  }
  int current_file = file;
  int current_rank = rank;
  while (true) {
    Square* sq = board->getSquare(current_file, current_rank);
    if (sq) {
      sq->setColor(square_color);
      sq->setType("NORMAL");
    }
    if (current_file == target_file && current_rank == target_rank) break;
    current_file += step_file;
    current_rank += step_rank;
  }
  return "";
}

std::vector<std::string> PainterRook::canSpecial(Board* board)
{ 
  std::vector<std::string> specials;
  int current_file = coordinates_.getFile();
  int current_rank = coordinates_.getRank();

  std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  for (const auto& [df, dr] : directions)
  {
    int target_file = current_file + df;
    int target_rank = current_rank + dr;
    while (target_file >= 0 && target_file < 8 && target_rank >= 0 && target_rank < 8)
    {
      Square* sq = board->getSquare(target_file, target_rank);
      if (!sq) break;
      Piece* p = sq->getPiece();
      if (p && p->getOwner() == owner_) break;

      specials.push_back("special " + std::string(1, 'a' + current_file) + 
      std::to_string(current_rank + 1) + " " +  std::string(1, 'a' + target_file) + 
      std::to_string(target_rank + 1));

      if (p) break;
      target_file += df;
      target_rank += dr;
    }
  }
  return specials;
}
