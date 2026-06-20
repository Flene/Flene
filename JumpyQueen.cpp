//----------------------------------------------------------------------------------------------------------------------
/// The JumpyQueen class represents a specialized queen variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "JumpyQueen.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include "Coordinates.hpp"

#include <cstdlib>

JumpyQueen::JumpyQueen(PlayerId owner)
  : Queen(owner)
{
  piece_id_ = "QJMP";
  short_name_ = (owner == PlayerId::WHITE) ? "♛Qj" : "♛qj";
}

bool JumpyQueen::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  return Queen::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string JumpyQueen::special(Player* active_player, int file, int rank,
                                std::vector<std::string> parameters, Board* board)
{
  if(active_player == nullptr || board == nullptr || parameters.size() < 2)
  {
    return "E_INVALID_MOVE";
  }

  Square* source_square = board->getSquare(file, rank);
  if(source_square == nullptr || source_square->getPiece() != this)
  {
    return "E_INVALID_MOVE";
  }

  Coordinates target(parameters.at(1));
  int target_file = target.getFile();
  int target_rank = target.getRank();

  Square* target_square = board->getSquare(target_file, target_rank);
  if(target_square == nullptr)
  {
    return "E_INVALID_MOVE";
  }

  int row_diff = std::abs(target_rank - rank);
  int col_diff = std::abs(target_file - file);
  if(!((row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2)))
  {
    return "E_INVALID_MOVE";
  }

  Piece* target_piece = target_square->getPiece();
  bool is_capture = target_piece != nullptr;
  if(target_piece != nullptr && target_piece->getOwner() == owner_)
  {
    return "E_INVALID_MOVE";
  }

  if(!movePieceOnBoard(rank, file, target_rank, target_file, is_capture, *board, *active_player, false, false))
  {
    return "E_INVALID_MOVE";
  }

  return "";
}

std::vector<std::string> JumpyQueen::canSpecial(Board* board)
{
  std::vector<std::string> specials;
  int file = coordinates_.getFile();
  int rank = coordinates_.getRank();
  std::vector<std::pair<int,int>> jumps =
  {
    { 1,  2}, { 2,  1},
    { 2, -1}, { 1, -2},
    {-1, -2}, {-2, -1},
    {-2,  1}, {-1,  2}
  };

  for (auto [df, dr] : jumps)
  {
    int tf = file + df;
    int tr = rank + dr;
    Square* target_sq = board->getSquare(tf, tr);
    if (!target_sq) continue;
    Piece* p = target_sq->getPiece();

    if (p && p->getOwner() == owner_) continue;
    std::string from = std::string(1, 'a' + file) + std::to_string(rank + 1);
    std::string to = std::string(1, 'a' + tf) + std::to_string(tr + 1);
    specials.push_back("special " + from + " " + to);
  }
  return specials;
}
