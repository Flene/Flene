//----------------------------------------------------------------------------------------------------------------------
/// The Player class represents a game participant, managing their resource pools (mana), elo ratings, 
/// the queue for pieces to place (placement phase), and captured opponent pieces (prison).
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>
#include <memory>
#include <string>

class Piece;

enum class PlayerId { WHITE = 1, BLACK}; // first player is white, second is black

class Player
{
  private:
    PlayerId id_;
    int mana_ = 0;
    int mana_pool_size_ = 0;
    int elo_score_ = 0;
    std::vector<std::unique_ptr<Piece>> piece_queue_; // used for start placement
    std::vector<std::unique_ptr<Piece>> prison_;      // captured pieces
    bool is_ai_ = false; // A3

  public:
    Player(PlayerId id);

    Player(const Player& other) = delete;
    ~Player();

    void addPieceToQueue(std::unique_ptr<Piece> piece);
    std::unique_ptr<Piece> popNextPiece();
    int countPieces(std::string piece_id);
    Piece* peekNextPiece() const;
    size_t getQueueSize() const;

    void addPieceToPrison(std::unique_ptr<Piece> piece);
    const std::vector<std::unique_ptr<Piece>>& getPrison() const;
    bool kingCaptured(PlayerId id);

    PlayerId getId();
    int getMana();
    int getEloScore();
    int getManaPoolSize();
    void setMana(int mana);
    void addMana(int amount);
    void setManaPoolSize(int mana_pool_size);
    void setEloScore(int elo_score);
    bool isAI() const { return is_ai_; } // A3
    void setIsAI(bool is_ai) { is_ai_ = is_ai; }


    std::unique_ptr<Piece> removeLastPrisonPiece();


    void restorePrison(std::vector<std::unique_ptr<Piece>> &&new_prison);
};

#endif