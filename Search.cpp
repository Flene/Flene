//----------------------------------------------------------------------------------------------------------------------
/// Implementation of SearchEngine (iterative deepening, negamax alpha-beta, quiescence, simple TT).
///
/// - Uses Game::generateAllLegalMoves / generateAllLegalSpecials to build move lists.
/// - Applies moves using Board::makeMoveSimulation and reverts using Board::undoMoveSimulation.
/// - Uses a simple string-key transposition table (not Zobrist) for portability.
/// - Time-limited single-threaded search (default 2000ms).
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Search.hpp"
#include "Game.hpp"
#include "Board.hpp"
#include "Player.hpp"
#include "Utils.hpp"
#include "Move.hpp"
#include <chrono>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <functional>
#include <string>

namespace {
  enum class TTFlag { EXACT, LOWERBOUND, UPPERBOUND };
  struct TTEntry {
    int value;
    int depth;
    TTFlag flag;
    std::string bestMove;
  };

  bool moveIsCapture(const std::string& mv, Board& board, Player& active) {
    if (mv.find('x') != std::string::npos) return true;
    std::vector<std::string> tokens;
    Utils::tokenize(mv, tokens, ' ');
    if (tokens.empty()) return false;
    
    // Special moves can be captures
    if (tokens[0] == "special") {
      for (int i = static_cast<int>(tokens.size()) - 1; i >= 1; --i) {
        if (Coordinates::isValid(tokens[i])) {
          Coordinates t(tokens[i]);
          Square* sq = board.getSquare(t.getFile(), t.getRank());
          if (!sq) return false;
          Piece* p = sq->getPiece();
          return (p != nullptr && p->getOwner() != active.getId());
        }
      }
    }
    // Regular moves are not captures (captures use 'x' notation)
    return false;
  }

  // Cheap string key for TT. Includes pieces, square types, side to move, turn_count and mana.
  std::string boardKey(Board& board, Player& active, Player& opponent, int turn_count) {
    std::string key;
    key.reserve(512);
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        Square* sq = board.getSquare(c, r);
        if (!sq) { key.push_back('.'); continue; }
        Piece* p = sq->getPiece();
        if (!p) { key.push_back('.'); continue; }
        key += p->getPieceId();
        key.push_back(p->getOwner() == PlayerId::WHITE ? 'W' : 'B');
        key.push_back(':');
      }
      key.push_back('|');
    }
    key += "#S#";
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        Square* sq = board.getSquare(c, r);
        if (!sq) continue;
        key += sq->getType();
        key.push_back(':');
        if (sq->hasItem()) key += sq->getItemId();
        key.push_back(';');
      }
    }
    key += "#A#";
    key.push_back(active.getId() == PlayerId::WHITE ? 'W' : 'B');
    key += std::to_string(turn_count);
    key += "#M#";
    key += std::to_string(active.getMana()) + "," + std::to_string(opponent.getMana());
    return key;
  }
}

SearchEngine::SearchEngine(Game& game, Board& board, int maxDepth, int timeLimitMs)
  : game_(game), board_(board), maxDepth_(maxDepth), timeLimitMs_(timeLimitMs) {}

std::string SearchEngine::findBestMove(Player& active, Player& opponent, int turn_count,
                                       bool frightened_king_cannot_capture,
                                       const std::set<std::string>& failed_commands)
{
  using clock = std::chrono::steady_clock;
  auto start_time = clock::now();
  auto end_time = start_time + std::chrono::milliseconds(timeLimitMs_);

  std::unordered_map<std::string, TTEntry> tt;
  std::vector<std::string> killer_moves(maxDepth_ + 1, "");

  auto checkGoldenPawnWin = [&](Player& currentPlayer) -> bool
  {
    int back_rank = (currentPlayer.getId() == PlayerId::WHITE) ? 7 : 0;
    for(int c = 0; c < 8; ++c)
    {
      Square* sq = board_.getSquare(c, back_rank);
      if(sq && sq->getPiece() && sq->getPiece()->getOwner() == currentPlayer.getId() && sq->getPiece()->getPieceId() == "PGLD")
      {
        return true;
      }
    }
    return false;
  };

  auto generateCandidates = [&](Player& p) {
    std::vector<std::string> m = game_.generateAllLegalMoves(p, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> s = game_.generateAllLegalSpecials(p, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> cand;
    cand.reserve(m.size() + s.size());
    for (auto &it : m) if (failed_commands.find(it) == failed_commands.end()) cand.push_back(it);
    for (auto &it : s) if (failed_commands.find(it) == failed_commands.end()) cand.push_back(it);
    if (cand.empty()) cand.push_back("pass");
    return cand;
  };

  auto evaluateStatic = [&](Player& a, Player& b) -> int {
    int score = 0;
    for (int rf = 0; rf < 8; ++rf)
    {
      for (int cf = 0; cf < 8; ++cf)
      {
        Square* sq = board_.getSquare(cf, rf);
        if (!sq) continue;
        Piece* p = sq->getPiece();
        if (!p) continue;
        int val = 0;
        switch (p->getType())
        {
          case PieceType::PAWN:   val = 100; break;
          case PieceType::KNIGHT: val = 320; break;
          case PieceType::BISHOP: val = 330; break;
          case PieceType::ROOK:   val = 500; break;
          case PieceType::QUEEN:  val = 900; break;
          case PieceType::KING:   val = 20000; break;
          default: val = 100; break;
        }
        // Special piece bonuses
        std::string pid = p->getPieceId();
        if (pid == "PGLD") val += 300;  // Golden pawn is valuable
        else if (pid == "PEXP") val += 150;  // Explosive pawn bonus
        else if (pid == "RINV") val += 150;  // Invincible rook bonus

        if (p->getOwner() == a.getId())
        {
          score += val;
          // Bonus for advanced pawns (closer to promotion)
          if (p->getType() == PieceType::PAWN) score += (a.getId() == PlayerId::WHITE) ? rf * 10 : (7 - rf) * 10;
        }
        else
        {
          score -= val;
          // Penalty for opponent advanced pawns
          if (p->getType() == PieceType::PAWN) score -= (b.getId() == PlayerId::WHITE) ? rf * 10 : (7 - rf) * 10;
        }
      }
    }
    // Mobility heuristic: more legal moves is better
    auto act_moves = game_.generateAllLegalMoves(a, turn_count, false, nullptr);
    auto opp_moves = game_.generateAllLegalMoves(b, turn_count, false, nullptr);
    score += static_cast<int>(act_moves.size()) * 5;
    score -= static_cast<int>(opp_moves.size()) * 5;
    
    // Mana advantage
    score += a.getMana() * 3;
    score -= b.getMana() * 3;
    
    return score;
  };

  // Quiescence search (captures + special-captures) to reduce horizon effect
  std::function<int(int,int,Player&,Player&)> quiescence;
  quiescence = [&](int alpha, int beta, Player& side, Player& other) -> int {
    if (clock::now() > end_time) return 0; // time cut
    int stand_pat = evaluateStatic(side, other);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    std::vector<std::string> caps;
    auto moves = game_.generateAllLegalMoves(side, turn_count, false, nullptr);
    auto specials = game_.generateAllLegalSpecials(side, turn_count, false, nullptr);
    for (auto &m : moves) if (m != "pass" && moveIsCapture(m, board_, side)) caps.push_back(m);
    for (auto &s : specials) if (s != "pass" && moveIsCapture(s, board_, side)) caps.push_back(s);

    std::sort(caps.begin(), caps.end(), [&](const std::string& a, const std::string& b){
      return game_.evaluateMove(a, side.getId()) > game_.evaluateMove(b, side.getId());
    });

    for (auto &mv : caps) {
      if (clock::now() > end_time) break;
      Board::UndoRecord rec = board_.makeMoveSimulation(mv, side, other, turn_count, frightened_king_cannot_capture);
      if (!rec.valid) continue;
      King* oppKing = board_.getKing(other.getId());
      bool pgld_win = false;
      if (mv.find("PGLD") != std::string::npos) pgld_win = checkGoldenPawnWin(side);
      int score;
      
      if (!oppKing || pgld_win) score = 100000;
      else score = -quiescence(-beta, -alpha, other, side);
      board_.undoMoveSimulation(rec, side, other);

      if (score >= beta) return beta;
      if (score > alpha) alpha = score;
    }
    return alpha;
  };

  // Negamax with alpha-beta pruning and transposition table
  std::function<int(int,int,int,Player&,Player&)> negamax;
  negamax = [&](int depth, int alpha, int beta, Player& side, Player& other) -> int 
  {
    if (clock::now() > end_time) return 0; // time up
    std::string key = boardKey(board_, side, other, turn_count);
    auto it = tt.find(key);
    if (it != tt.end() && it->second.depth >= depth) {
      TTEntry const &ent = it->second;
      if (ent.flag == TTFlag::EXACT) return ent.value;
      if (ent.flag == TTFlag::LOWERBOUND) alpha = std::max(alpha, ent.value);
      else if (ent.flag == TTFlag::UPPERBOUND) beta = std::min(beta, ent.value);
      if (alpha >= beta) return ent.value;
    }

    if (depth == 0) {
      return quiescence(alpha, beta, side, other);
    }

    std::vector<std::string> moves = game_.generateAllLegalMoves(side, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> specials = game_.generateAllLegalSpecials(side, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> cand;
    cand.insert(cand.end(), moves.begin(), moves.end());
    cand.insert(cand.end(), specials.begin(), specials.end());

    if (cand.empty()) {
      return evaluateStatic(side, other);
    }

    std::string killer = killer_moves[depth];

    // Sort with killer moves first for better pruning
    std::sort(cand.begin(), cand.end(), [&](const std::string& a, const std::string& b)
    {
      if (a == killer) return true;
      if (b == killer) return false;
      return game_.evaluateMove(a, side.getId()) > game_.evaluateMove(b, side.getId());
    });

    int bestValue = std::numeric_limits<int>::min() / 4;
    std::string bestMoveLocal;

    for (const auto &mv : cand)
    {
      if (clock::now() > end_time) break;

      if (mv == "pass")
      {
        int val = -negamax(depth - 1, -beta, -alpha, other, side) - 10;
        if (val > bestValue) { bestValue = val; bestMoveLocal = mv; }
        alpha = std::max(alpha, bestValue);
        if (alpha >= beta) break;
        continue;
      }

      Board::UndoRecord rec = board_.makeMoveSimulation(mv, side, other, turn_count, frightened_king_cannot_capture);
      if (!rec.valid) continue;

      // Recursive alpha-beta search
      King* oppKing = board_.getKing(other.getId());
      bool pgld_win = false;
      if (mv.find("PGLD") != std::string::npos) pgld_win = checkGoldenPawnWin(side);
      int score = (!oppKing || pgld_win) ? 100000 : -negamax(depth - 1, -beta, -alpha, other, side);

      board_.undoMoveSimulation(rec, side, other);

      if (score > bestValue)
      {
        bestValue = score;
        bestMoveLocal = mv;
        // Alpha-beta cutoff update
        if (bestValue > alpha)
        {
          alpha = bestValue;
          // Store non-capture moves as killers (quiet moves more likely to be useful)
          if (!moveIsCapture(mv, board_, side)) killer_moves[depth] = mv;
        }
      }
      if (alpha >= beta) break;  // Beta cutoff
    }

    TTEntry entry;
    entry.value = bestValue;
    entry.depth = depth;
    entry.bestMove = bestMoveLocal;
    if (bestValue <= alpha) entry.flag = TTFlag::UPPERBOUND;
    else if (bestValue >= beta) entry.flag = TTFlag::LOWERBOUND;
    else entry.flag = TTFlag::EXACT;
    tt[key] = std::move(entry);

    return bestValue;
  };

  // Iterative deepening with root move ordering
  std::vector<std::string> root_candidates = generateCandidates(active);
  // Initial sort by quick heuristic
  std::sort(root_candidates.begin(), root_candidates.end(), [&](const std::string& a, const std::string& b){
    return game_.evaluateMove(a, active.getId()) > game_.evaluateMove(b, active.getId());
  });

  std::string best_move = root_candidates.front();
  int best_score = std::numeric_limits<int>::min();

  for (int depth = 1; depth <= maxDepth_; ++depth) {
    if (clock::now() > end_time) break;

    int best_score_this = std::numeric_limits<int>::min();
    std::string best_move_this = best_move;

    // Reorder root moves using TT history
    std::string rootKey = boardKey(board_, active, opponent, turn_count);
    auto rt = tt.find(rootKey);
    std::string rtbest = (rt != tt.end()) ? rt->second.bestMove : "";
    if (!rtbest.empty()) {
      auto it = std::find(root_candidates.begin(), root_candidates.end(), rtbest);
      if (it != root_candidates.end()) {
        std::iter_swap(root_candidates.begin(), it);
      }
    }

    for (const auto &mv : root_candidates) {
      if (clock::now() > end_time) break;

      if (mv == "pass") {
        int val = -negamax(depth - 1, std::numeric_limits<int>::min()/4, std::numeric_limits<int>::max()/4, opponent, active) - 10;
        if (val > best_score_this) { best_score_this = val; best_move_this = mv; }
        continue;
      }

      Board::UndoRecord rec = board_.makeMoveSimulation(mv, active, opponent, turn_count, frightened_king_cannot_capture);
      if (!rec.valid) continue;
      King* oppKing = board_.getKing(opponent.getId());
      bool pgld_win = false;
      if (mv.find("PGLD") != std::string::npos) pgld_win = checkGoldenPawnWin(active);
      int val;
      
      if (!oppKing || pgld_win) val = 100000;
      else val = -negamax(depth - 1, std::numeric_limits<int>::min()/4, std::numeric_limits<int>::max()/4, opponent, active);
      board_.undoMoveSimulation(rec, active, opponent);

      if (val > best_score_this) {
        best_score_this = val;
        best_move_this = mv;
      }
    }

    if (clock::now() > end_time) break;

    // Adopt depth result
    best_move = best_move_this;
    best_score = best_score_this;

    if (best_score >= 90000) break;  // Mate found, no need to search deeper
  }

  return best_move;
}
