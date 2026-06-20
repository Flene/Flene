//----------------------------------------------------------------------------------------------------------------------
/// The Square class represents a single tile on the chess board, managing its background color, 
/// the piece currently occupying it, and item interactions or spawn logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef SQUARE_HPP
#define SQUARE_HPP

#include <vector>
#include <memory>
#include <string>

class Piece;
class Player;

enum class SquareColor { WHITE = 1, BLACK};

class Square
{
  protected:
    std::unique_ptr<Piece> piece_= nullptr;
    std::string type_ = "NORMAL";
    SquareColor color_;
    std::string item_id_;

  public:
    Square(SquareColor color);
    virtual ~Square();

    SquareColor getColor() const { return color_; } // white and black even for special squares for move logic
    std::string getType() const { return type_; } // for color in boardPrint
    void setColor(SquareColor color) { color_ = color; }
    void setType(const std::string& type) { type_ = type; }

    void setPiece(std::unique_ptr<Piece> piece) { piece_ = std::move(piece); }
    Piece* getPiece() { return piece_.get(); }
    const Piece* getPiece() const { return piece_.get(); }
    
    bool isEmpty() const { return piece_ == nullptr; }

    bool hasItem() const { return !item_id_.empty(); }
    const std::string& getItemId() const { return item_id_; }
    std::string getItemDisplayName() const;
    void setItem(const std::string& item_id) { item_id_ = item_id; }
    void clearItem() { item_id_.clear(); }

    virtual void addSpawnItem(const std::string& item_id) { (void)item_id; }
    virtual bool spawnNextItem();
    virtual void executeEffect(Player& player, Piece& piece); // in mana-, boost- and spawn square
    std::unique_ptr<Piece> releasePiece(); // if piece is captured
};

#endif
