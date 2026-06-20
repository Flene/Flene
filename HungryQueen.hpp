//----------------------------------------------------------------------------------------------------------------------
/// The HungryQueen class represents a specialized queen variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef HUNGRYQUEEN_HPP
#define HUNGRYQUEEN_HPP

#include "Queen.hpp"

class HungryQueen : public Queen
{
  public:
    HungryQueen(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
};

#endif
