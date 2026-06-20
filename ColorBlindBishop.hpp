//----------------------------------------------------------------------------------------------------------------------
/// The ColorBlindBishop class represents a specialized bishop variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef COLORBLINDBISHOP_HPP
#define COLORBLINDBISHOP_HPP

#include "Bishop.hpp"
#include "Board.hpp"

class ColorBlindBishop : public Bishop
{
  public:
    ColorBlindBishop(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
    virtual std::string special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board) override;
    virtual bool hasSpecial() override { return true; };
    virtual std::vector<std::string> canSpecial(Board* board) override;
};

#endif
