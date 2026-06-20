//----------------------------------------------------------------------------------------------------------------------
/// The ManaSquare class represents a specialized mana square tile, handling the logic to 
/// grant additional resource points (mana) to a player when a piece moves onto it.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef MANASQUARE_HPP
#define MANASQUARE_HPP

#include "Square.hpp"

class ManaSquare : public Square {
public:
    ManaSquare(SquareColor color);

    void executeEffect(Player& player, Piece& piece) override;
};

#endif
