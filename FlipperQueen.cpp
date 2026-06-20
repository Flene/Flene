//----------------------------------------------------------------------------------------------------------------------
/// The FlipperQueen class represents a specialized queen variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "FlipperQueen.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include "Coordinates.hpp"

#include <cstdlib>

namespace
{
  int signOf(int value)
  {
    if(value > 0) return 1;
    if(value < 0) return -1;
    return 0;
  }

  bool isBoardEdge(int file, int rank)
  {
    return file == 0 || file == 7 || rank == 0 || rank == 7;
  }

  bool isCorner(int file, int rank)
  {
    return (file == 0 || file == 7) && (rank == 0 || rank == 7);
  }

  bool diagonalFromTo(int start_file, int start_rank, int target_file, int target_rank)
  {
    int file_diff = std::abs(target_file - start_file);
    int rank_diff = std::abs(target_rank - start_rank);
    return file_diff == rank_diff && file_diff > 0;
  }

  bool pathSegmentClear(Board& board, int start_file, int start_rank,
                        int target_file, int target_rank, bool include_target)
  {
    int file_step = signOf(target_file - start_file);
    int rank_step = signOf(target_rank - start_rank);
    int current_file = start_file + file_step;
    int current_rank = start_rank + rank_step;

    while(current_file != target_file || current_rank != target_rank)
    {
      Square* square = board.getSquare(current_file, current_rank);
      if(square == nullptr || square->getPiece() != nullptr) return false;
      current_file += file_step;
      current_rank += rank_step;
    }

    if(include_target)
    {
      Square* target_square = board.getSquare(target_file, target_rank);
      if(target_square == nullptr || target_square->getPiece() != nullptr) return false;
    }
    return true;
  }
}

FlipperQueen::FlipperQueen(PlayerId owner)
  : Queen(owner)
{
  piece_id_ = "QFLP";
  short_name_ = (owner == PlayerId::WHITE) ? "♛Qf" : "♛qf";
}

bool FlipperQueen::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Queen::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string FlipperQueen::special(Player* active_player, int file, int rank,
                                  std::vector<std::string> parameters, Board* board)
{
  if(active_player == nullptr || board == nullptr || parameters.size() != 3)
  {
    return "E_INVALID_MOVE";
  }

  Coordinates bounce_coordinates(parameters.at(1));
  Coordinates target_coordinates(parameters.at(2));
  int bounce_file = bounce_coordinates.getFile();
  int bounce_rank = bounce_coordinates.getRank();
  int target_file = target_coordinates.getFile();
  int target_rank = target_coordinates.getRank();

  Square* source_square = board->getSquare(file, rank);
  Square* bounce_square = board->getSquare(bounce_file, bounce_rank);
  Square* target_square = board->getSquare(target_file, target_rank);
  if(source_square == nullptr || bounce_square == nullptr || target_square == nullptr)
  {
    return "E_INVALID_MOVE";
  }
  if(source_square->getPiece() != this)
  {
    return "E_INVALID_MOVE";
  }

  if(!isBoardEdge(bounce_file, bounce_rank) || isCorner(bounce_file, bounce_rank))
  {
    return "E_INVALID_MOVE";
  }

  if(!diagonalFromTo(file, rank, bounce_file, bounce_rank))
  {
    return "E_INVALID_MOVE";
  }

  bool bounce_on_vertical_edge = (bounce_file == 0 || bounce_file == 7);
  int incoming_file_step = signOf(bounce_file - file);
  int incoming_rank_step = signOf(bounce_rank - rank);
  int outgoing_file_step = bounce_on_vertical_edge ? -incoming_file_step : incoming_file_step;
  int outgoing_rank_step = bounce_on_vertical_edge ? incoming_rank_step : -incoming_rank_step;

  int after_bounce_file_diff = target_file - bounce_file;
  int after_bounce_rank_diff = target_rank - bounce_rank;
  int after_bounce_distance = std::abs(after_bounce_file_diff);
  if(after_bounce_distance == 0 || std::abs(after_bounce_rank_diff) != after_bounce_distance)
  {
    return "E_INVALID_MOVE";
  }
  if(signOf(after_bounce_file_diff) != outgoing_file_step ||
     signOf(after_bounce_rank_diff) != outgoing_rank_step)
  {
    return "E_INVALID_MOVE";
  }

  if(!pathSegmentClear(*board, file, rank, bounce_file, bounce_rank, true))
  {
    return "E_INVALID_MOVE";
  }

  if(!pathSegmentClear(*board, bounce_file, bounce_rank, target_file, target_rank, false))
  {
    return "E_INVALID_MOVE";
  }

  Piece* target_piece = target_square->getPiece();
  bool is_capture = target_piece != nullptr;
  if(target_piece != nullptr && target_piece->getOwner() == active_player->getId())
  {
    return "E_INVALID_MOVE";
  }

  if(!movePieceOnBoard(rank, file, target_rank, target_file, is_capture, *board, *active_player))
  {
    return "E_INVALID_MOVE";
  }
  return "";
}

std::vector<std::string> FlipperQueen::canSpecial(Board* board)
{
  std::vector<std::string> specials;

  int file = coordinates_.getFile();
  int rank = coordinates_.getRank();
  std::vector<std::pair<int,int>> diagonals = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};

  for (auto [df, dr] : diagonals)
  {
    int bf = file + df;
    int br = rank + dr;
    while (bf >= 0 && bf < 8 && br >= 0 && br < 8)
    {
      if (isBoardEdge(bf, br) && !isCorner(bf, br))
      {
        if (!pathSegmentClear(*board, file, rank, bf, br, true)) break;
        bool bounce_on_vertical = (bf == 0 || bf == 7);
        int incoming_df = signOf(bf - file);
        int incoming_dr = signOf(br - rank);

        int outgoing_df = bounce_on_vertical ? -incoming_df : incoming_df;
        int outgoing_dr = bounce_on_vertical ?  incoming_dr : -incoming_dr;

        int tf = bf + outgoing_df;
        int tr = br + outgoing_dr;

        while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8)
        {
          if (!pathSegmentClear(*board, bf, br, tf, tr, false)) break;
          Square* target_sq = board->getSquare(tf, tr);
          Piece* target_piece = target_sq->getPiece();
          if (target_piece && target_piece->getOwner() == owner_) break;

          std::string from = std::string(1, 'a' + file) + std::to_string(rank + 1);
          std::string bounce = std::string(1, 'a' + bf) + std::to_string(br + 1);

          std::string to = std::string(1, 'a' + tf) + std::to_string(tr + 1);
          specials.push_back("special " + from + " " + bounce + " " + to);
          // Stop after first target in this direction
          break;

          tf += outgoing_df;
          tr += outgoing_dr;
        }
      }
      bf += df;
      br += dr;
    }
  }
  return specials;
}
