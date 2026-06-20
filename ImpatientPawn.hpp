//----------------------------------------------------------------------------------------------------------------------
/// The ImpatientPawn class represents a specialized pawn variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef IMPATIENTPAWN_HPP
#define IMPATIENTPAWN_HPP

#include "Pawn.hpp"
#include "Utils.hpp"

class ImpatientPawn : public Pawn
{
  public:
    ImpatientPawn(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
    virtual std::string special(Player* active_player, int file, int rank, std::vector<std::string> parameters, Board* board) override;
    virtual bool hasSpecial() override { return true; };
    virtual std::vector<std::string> canSpecial(Board*) override;
};

#endif
