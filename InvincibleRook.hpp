//----------------------------------------------------------------------------------------------------------------------
/// The InvincibleRook class represents a specialized rook variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef INVINCIBLEROOK_HPP
#define INVINCIBLEROOK_HPP

#include "Rook.hpp"
#include "Board.hpp"

class InvincibleRook : public Rook
{
  public:
    InvincibleRook(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
    virtual std::string special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board) override;
    virtual bool hasSpecial() override { return true; };
    virtual std::vector<std::string> canSpecial(Board*) override;
};

#endif
