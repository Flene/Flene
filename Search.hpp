//----------------------------------------------------------------------------------------------------------------------
/// SearchEngine - iterative deepening + negamax + alpha-beta + quiescence + simple TT wrapper used by Game.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <string>
#include <set>

class Game;
class Board;
class Player;

class SearchEngine
{
  public:
    SearchEngine(Game& game, Board& board, int maxDepth = 6, int timeLimitMs = 2000);

    std::string findBestMove(Player& active, Player& opponent, int turn_count,
                             bool frightened_king_cannot_capture,
                             const std::set<std::string>& failed_commands);

  private:
    Game& game_;
    Board& board_;
    int maxDepth_;
    int timeLimitMs_;
};

#endif // SEARCH_HPP
