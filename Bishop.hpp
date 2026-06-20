//----------------------------------------------------------------------------------------------------------------------
/// The Bishop class represents a standard bishop piece and implements the specific movement logic, 
/// restricted by its color-bound square rules.
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef BISHOP_HPP
#define BISHOP_HPP

#include "Piece.hpp"
#include "Player.hpp"
#include "Square.hpp"

class Bishop : public Piece
{
  private:
    SquareColor inherent_color_;
  public:
    Bishop(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col, 
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;
    void initializeOnSquare(Square* square) override;
    void setInherentColor(SquareColor color) { inherent_color_ = color; }
    SquareColor getInherentColor() const { return inherent_color_; }
};

#endif
