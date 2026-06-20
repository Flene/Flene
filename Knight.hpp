//----------------------------------------------------------------------------------------------------------------------
/// The Knight class represents a standard knight piece and implements the specific movement logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef KNIGHT_HPP
#define KNIGHT_HPP

#include "Piece.hpp"
#include "Player.hpp"

class Knight : public Piece
{
  public:
    Knight(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
};

#endif
