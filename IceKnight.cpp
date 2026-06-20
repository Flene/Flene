//----------------------------------------------------------------------------------------------------------------------
/// The IceKnight class represents a specialized knight variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "IceKnight.hpp"
#include "Player.hpp"

IceKnight::IceKnight(PlayerId owner)
  : Knight(owner)
{
  piece_id_ = "NICE";
  short_name_ = (owner == PlayerId::WHITE) ? "♞Ni" : "♞ni";
}

void IceKnight::freezePiece(int col, int row, Board& board, PlayerId active_owner)
{
  Square* square = board.getSquare(col, row);
  if(square == nullptr) return;
  Piece* piece = square->getPiece();
  if(piece == nullptr) return;

  int turns = (piece->getOwner() == active_owner) ? 2 : 1;
  piece->setFrozenTurns(turns);
}

bool IceKnight::move(int start_rank, int start_file, int target_row, int target_col, 
                      bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  int row_diff = std::abs(target_row - start_rank);
  int col_diff = std::abs(target_col - start_file);
  if(!((row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2))) return false;

  bool moved = movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
  if(!moved || dry_run) return moved;

  if(target_row - start_rank == 2)
  {
    freezePiece(start_file, start_rank + 1, board, active_player.getId());
    freezePiece(start_file, start_rank + 2, board, active_player.getId());
  }
  else if(target_col - start_file == 2)
  {
    freezePiece(start_file + 1, start_rank, board, active_player.getId());
    freezePiece(start_file + 2, start_rank, board, active_player.getId());
  }
  else if(target_row - start_rank == -2)
  {
    freezePiece(start_file, start_rank - 1, board, active_player.getId());
    freezePiece(start_file, start_rank - 2, board, active_player.getId());
  }
  else if(target_col - start_file == -2)
  {
    freezePiece(start_file - 1, start_rank, board, active_player.getId());
    freezePiece(start_file - 2, start_rank, board, active_player.getId());
  }
  return true;
}
