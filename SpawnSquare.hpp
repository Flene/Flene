//----------------------------------------------------------------------------------------------------------------------
/// The SpawnSquare class represents a specialized spawn square tile, managing a queued list of items 
/// and handling item generation logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef SPAWNSQUARE_HPP
#define SPAWNSQUARE_HPP

#include "Square.hpp"
#include <vector>
#include <string>

class SpawnSquare : public Square
{
  private:
    std::vector<std::string> spawn_items_;
    std::size_t next_item_index_ = 0;

  public:
    SpawnSquare(SquareColor color);

    void addSpawnItem(const std::string& item_id) override;
    bool spawnNextItem() override;
    void executeEffect(Player& player, Piece& piece) override;
};

#endif
