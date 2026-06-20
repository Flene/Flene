//----------------------------------------------------------------------------------------------------------------------
/// The Player class represents a game participant, managing their resource pools (mana), elo ratings, 
/// the queue for pieces to place (placement phase), and captured opponent pieces (prison).
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#include "Player.hpp"
#include "Piece.hpp"
#include <algorithm>

Player::Player(PlayerId id) : id_(id) {}

void Player::addPieceToQueue(std::unique_ptr<Piece> piece)
{
  if(piece) piece_queue_.push_back(std::move(piece)); // move cause can't copy unique ptr
}

Player::~Player() = default;

std::unique_ptr<Piece> Player::popNextPiece()
{
  if (piece_queue_.empty()) return nullptr;
  auto piece = std::move(piece_queue_.front()); // move cause can't copy unique ptr
  piece_queue_.erase(piece_queue_.begin());
  return piece;
}

int Player::countPieces(std::string piece_id)
{
  int count = 0;
  for(const auto& p : piece_queue_)
  {
    if(p && p->getPieceId() == piece_id) count++;
  }
  return count;
}

Piece* Player::peekNextPiece() const
{
  return piece_queue_.empty() ? nullptr : piece_queue_.front().get();
}

size_t Player::getQueueSize() const { return piece_queue_.size(); }

void Player::addPieceToPrison(std::unique_ptr<Piece> piece)
{
  if(piece)
  {
    piece->setCaptured(true);
    prison_.push_back(std::move(piece));
  }
}

const std::vector<std::unique_ptr<Piece>>& Player::getPrison() const
{
  return prison_;
}

bool Player::kingCaptured(PlayerId id)
{
  for (const auto& p : prison_)
  {
    if(p->getType() == PieceType::KING && p->getOwner() == id)
    {
      return true;
    }
  }
  return false;
}

PlayerId Player::getId() { return id_; }

int Player::getMana() { return mana_; }

int Player::getEloScore() { return elo_score_; }

int Player::getManaPoolSize() { return mana_pool_size_; }

void Player::setMana(int mana)
{
  mana_ = std::max(0, mana);
  if(mana_pool_size_ > 0) mana_ = std::min(mana_, mana_pool_size_);
}

void Player::addMana(int amount)
{
  setMana(mana_ + amount);
}

void Player::setManaPoolSize(int mana_pool_size)
{
  mana_pool_size_ = std::max(0, mana_pool_size);
  setMana(mana_);
}

void Player::setEloScore(int elo_score)
{
  elo_score_ = elo_score;
}

std::unique_ptr<Piece> Player::removeLastPrisonPiece()
{
  if (prison_.empty()) return nullptr;
  auto piece = std::move(prison_.back());
  prison_.pop_back();
  if (piece) piece->setCaptured(false);
  return piece;
}

void Player::restorePrison(std::vector<std::unique_ptr<Piece>> &&new_prison)
{
  // replace prison contents with provided vector
  prison_.clear();
  prison_ = std::move(new_prison);
  // ensure captured flag is set for every piece in prison
  for (auto& p : prison_)
  {
    if (p) p->setCaptured(true);
  }
}