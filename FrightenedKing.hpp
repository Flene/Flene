//----------------------------------------------------------------------------------------------------------------------
/// The FrightenedKing class represents a specialized king variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef FRIGHTENEDKING_HPP
#define FRIGHTENEDKING_HPP

#include "King.hpp"

class FrightenedKing : public King
{
  public:
    FrightenedKing(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
};

#endif
