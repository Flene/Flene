//----------------------------------------------------------------------------------------------------------------------
/// The Pawn class represents a standard pawn piece, implementing specific movement logic, 
/// capture rules, en passant logic, and promotion handling.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef PAWN_HPP
#define PAWN_HPP

#include "Piece.hpp"
#include "Rook.hpp"
#include "Knight.hpp"
#include "Bishop.hpp"
#include "Queen.hpp"

class Pawn : public Piece
{
  public:
    Pawn(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;

    virtual void promotion(const std::string& type, Board& board, int file, int rank);
};

#endif
