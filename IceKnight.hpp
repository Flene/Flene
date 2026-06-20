//----------------------------------------------------------------------------------------------------------------------
/// The IceKnight class represents a specialized knight variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef ICEKNIGHT_HPP
#define ICEKNIGHT_HPP

#include "Knight.hpp"
#include "Board.hpp"

class IceKnight : public Knight
{
  public:
    IceKnight(PlayerId owner);

    void freezePiece(int col, int row, Board& board, PlayerId active_owner);
    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
};

#endif
