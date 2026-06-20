//----------------------------------------------------------------------------------------------------------------------
/// The Rook class represents a standard rook piece and implements the specific movement logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Rook.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include <cstdlib>

Rook::Rook(PlayerId owner)
    : Piece(PieceType::ROOK, owner, "R", (owner == PlayerId::WHITE ? "♜R" : "♜r"), 5)
{}

bool Rook::move(int start_rank, int start_file, int target_row, int target_col, 
                bool is_capture, Board& board, Player& active_player, bool dry_run) 
{
  if(start_rank != target_row && start_file != target_col) return false;
  if(start_rank == target_row && start_file == target_col) return false;
  if(!isPathClear(start_rank, start_file, target_row, target_col, board)) return false;
  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
}
