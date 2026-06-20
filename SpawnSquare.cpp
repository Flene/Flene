//----------------------------------------------------------------------------------------------------------------------
/// The SpawnSquare class represents a specialized spawn square tile, managing a queued list of items 
/// and handling item generation logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "SpawnSquare.hpp"
#include "Piece.hpp"
#include "Player.hpp"

SpawnSquare::SpawnSquare(SquareColor color)
  : Square(color) { type_ = "SPAWN"; }

void SpawnSquare::addSpawnItem(const std::string& item_id)
{
  spawn_items_.push_back(item_id);
}

bool SpawnSquare::spawnNextItem()
{
  if(spawn_items_.empty() || hasItem()) return false;

  Piece* standing_piece = getPiece();
  if(standing_piece != nullptr && standing_piece->hasItem()) return false;

  const std::string item = spawn_items_.at(next_item_index_);
  next_item_index_ = (next_item_index_ + 1) % spawn_items_.size();

  if(standing_piece != nullptr)
  {
    standing_piece->setItem(item);
  }
  else
  {
    setItem(item);
  }
  return true;
}

void SpawnSquare::executeEffect(Player& player, Piece& piece)
{
  Square::executeEffect(player, piece);
}
