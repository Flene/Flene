//----------------------------------------------------------------------------------------------------------------------
/// The ExplosivePawn class represents a specialized pawn variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "ExplosivePawn.hpp"
#include "Player.hpp"

ExplosivePawn::ExplosivePawn(PlayerId owner)
  : Pawn(owner)
{
  piece_id_ = "PEXP";
  short_name_ = (owner == PlayerId::WHITE) ? "♟P!" : "♟p!";
}

bool ExplosivePawn::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
    return Pawn::move(start_rank, start_file, target_row, target_col, is_capture, board, active_player, dry_run);
}

std::string ExplosivePawn::special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board)
{
  Coordinates target_coords(parameters.at(1));
  int target_file = target_coords.getFile();
  int target_rank = target_coords.getRank();

  int row_diff = target_rank - rank;
  int col_diff = target_file - file;
  int direction = (owner_ == PlayerId::WHITE) ? 1 : -1;

  if(row_diff != direction || std::abs(col_diff) != 1) return "E_INVALID_MOVE";

  Square* target_square = board->getSquare(target_file, target_rank);
  if(!target_square) return "E_INVALID_MOVE";

  Piece* target_piece = target_square->getPiece();
  if(!target_piece || target_piece->getOwner() == owner_) return "E_INVALID_MOVE";

  Square* source_square = board->getSquare(file, rank);
  bool pawn_moved = true;

  if(target_piece->hasItem("SHIELD"))
  {
    target_piece->clearItem();
    pawn_moved = false;
  }
  else
  {
    bool return_value = Pawn::move(rank, file, target_rank, target_file, true, *board, *active_player);
    if(!return_value) return "E_INVALID_MOVE";
  }

  int center_file = pawn_moved ? target_file : file;
  int center_rank = pawn_moved ? target_rank : rank;

  for(int dr = -1; dr <= 1; dr++)
  {
    for(int dc = -1; dc <= 1; dc++)
    {
      if(dr == 0 && dc == 0) continue;

      Square* sq = board->getSquare(center_file + dc, center_rank + dr);
      if(!sq) continue;

      Piece* p = sq->getPiece();
      if(!p) continue;

      if(p->getType() == PieceType::PAWN || p->hasItem("SHIELD"))
      {
        p->clearItem();
        continue;
      }

      std::unique_ptr<Piece> captured = sq->releasePiece();
      active_player->addPieceToPrison(std::move(captured));
    }
  }

  if(!pawn_moved)
  {
    Piece* remaining = target_square->getPiece();
    if(remaining)
    {
      if(remaining->getType() == PieceType::PAWN)
      {
        remaining->clearItem();
      }
      else
      {
        remaining->clearItem();
        std::unique_ptr<Piece> captured = target_square->releasePiece();
        active_player->addPieceToPrison(std::move(captured));
      }
    }
  }

  Square* pawn_square = pawn_moved ? target_square : source_square;
  Piece* pawn = pawn_square->getPiece();
  if(pawn)
  {
    std::unique_ptr<Piece> exploded = pawn_square->releasePiece();
    active_player->addPieceToPrison(std::move(exploded));
  }

  return "";
}


std::vector<std::string> ExplosivePawn::canSpecial(Board* board)
{
  std::vector<std::string> specials;
  int current_file = coordinates_.getFile();
  int current_rank = coordinates_.getRank();

  int direction = (owner_ == PlayerId::WHITE) ? 1 : -1;
  int target_rank = current_rank + direction;
  for(int dc = -1; dc <= 1; dc++)
  {
    if(dc == 0) continue;
    int target_file = current_file + dc;
    Square* target_square = board->getSquare(target_file, target_rank);
    if(!target_square) continue;
    Piece* target_piece = target_square->getPiece();
    if(!target_piece || target_piece->getOwner() == owner_) continue;

    specials.push_back("special " + std::string(1, 'a' + current_file) + 
    std::to_string(current_rank + 1) + " " + std::string(1, 'a' + target_file) + 
    std::to_string(target_rank + 1));
  }
  return specials;
}

