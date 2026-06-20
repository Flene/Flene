//----------------------------------------------------------------------------------------------------------------------
/// The GoldenPawn class represents a specialized pawn variant and executes its unique special ability.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef GOLDENPAWN_HPP
#define GOLDENPAWN_HPP

#include "Pawn.hpp"

class GoldenPawn : public Pawn
{
  public:
    GoldenPawn(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
              bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
    void promotion(const std::string& type, Board& board, int file, int rank) override;
};

#endif
