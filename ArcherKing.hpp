//----------------------------------------------------------------------------------------------------------------------
/// The ArcherKing class represents a specialized king variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef ARCHERKING_HPP
#define ARCHERKING_HPP

#include "King.hpp"

class ArcherKing : public King
{
  public:
    ArcherKing(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;

    std::string special(Player* active_player, int file, int rank,
                        std::vector<std::string> parameters, Board* board) override;
    virtual bool hasSpecial() override { return true; };
    std::vector<std::string> canSpecial(Board* board) override;
};

#endif
