//----------------------------------------------------------------------------------------------------------------------
/// The ManaSquare class represents a specialized mana square tile, handling the logic to 
/// grant additional resource points (mana) to a player when a piece moves onto it.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "ManaSquare.hpp"
#include "Player.hpp"
#include "Piece.hpp"

ManaSquare::ManaSquare(SquareColor color)
    : Square(color) { type_ = "MANA"; }

void ManaSquare::executeEffect(Player& player, Piece& piece)
{
  player.addMana(1);
  Square::executeEffect(player, piece);
}
