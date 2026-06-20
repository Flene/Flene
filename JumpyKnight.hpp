//----------------------------------------------------------------------------------------------------------------------
/// The JumpyKnight class represents a specialized knight variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef JUMPYKNIGHT_HPP
#define JUMPYKNIGHT_HPP

#include "Knight.hpp"

class JumpyKnight : public Knight
{
  public:
    JumpyKnight(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
};

#endif
