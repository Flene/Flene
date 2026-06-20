//----------------------------------------------------------------------------------------------------------------------
/// The BoostSquare class represents a specialized boost square tile, triggering an immediate additional 
/// move for the owner of the occupying piece.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "BoostSquare.hpp"
#include "Piece.hpp"
#include "Player.hpp"

BoostSquare::BoostSquare(SquareColor color)
  : Square(color) { type_ = "BOOST"; }

void BoostSquare::executeEffect(Player& player, Piece& piece)
{
  Square::executeEffect(player, piece);
}
