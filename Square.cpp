//----------------------------------------------------------------------------------------------------------------------
/// The Square class represents a single tile on the chess board, managing its background color, 
/// the piece currently occupying it, and item interactions or spawn logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Square.hpp"
#include "Piece.hpp"
#include "Player.hpp"

Square::Square(SquareColor color)
  : color_(color) {}

Square::~Square() = default;

std::string Square::getItemDisplayName() const
{
  if(item_id_ == "FREEZE") return "*";
  if(item_id_ == "TP") return "→";
  if(item_id_ == "EVENODD") return "½";
  if(item_id_ == "LUKE") return "↑";
  if(item_id_ == "SHIELD") return "□";
  if(item_id_ == "CLOAK") return "⌂";
  if(item_id_ == "REPEL") return "R";
  return " ";
}

bool Square::spawnNextItem()
{
  return false;
}

void Square::executeEffect(Player& player, Piece& piece)
{
  (void)player;
  if(hasItem())
  {
    piece.setItem(item_id_);
    clearItem();
  }
}

std::unique_ptr<Piece> Square::releasePiece()
{
  return std::move(piece_);
}
