//----------------------------------------------------------------------------------------------------------------------
/// The Queen class represents a standard queen piece and implements the specific movement logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef QUEEN_HPP
#define QUEEN_HPP

#include "Piece.hpp"
#include "Player.hpp"

class Queen : public Piece
{
  protected:
    bool allow_friendly_capture_ = false;
  public:
    Queen(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
};

#endif
