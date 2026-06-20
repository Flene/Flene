//----------------------------------------------------------------------------------------------------------------------
/// The BoostSquare class represents a specialized boost square tile, triggering an immediate additional 
/// move for the owner of the occupying piece.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef BOOSTSQUARE_HPP
#define BOOSTSQUARE_HPP

#include "Square.hpp"
#include <vector>
#include <memory>

class BoostSquare : public Square
{
  public:
    BoostSquare(SquareColor color);

    void executeEffect(Player& player, Piece& piece) override;
};

#endif
