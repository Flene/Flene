//----------------------------------------------------------------------------------------------------------------------
/// Implementation of SearchEngine: iterative deepening, negamax alpha-beta, quiescence, Zobrist-based TT,
/// 2-level killer moves, history heuristic, MVV/LVA move ordering, LMR, check extension, null move pruning,
/// aspiration windows, PST, pawn structure evaluation, king safety, endgame enhancements, opening book,
/// and improved time management.
///
/// Note on parallel search (SMP): Board is non-copyable by design, which prevents independent per-thread
/// board states. SMP would require Board to expose a deep-copy/clone interface; the infrastructure for it
/// is documented here but not activated.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Search.hpp"
#include "Game.hpp"
#include "Board.hpp"
#include "Player.hpp"
#include "Utils.hpp"
#include "Move.hpp"
#include "King.hpp"
#include <chrono>
#include <unordered_map>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>
#include <functional>
#include <string>
#include <cstdint>

namespace {

  // ---------------------------------------------------------------------------
  // Transposition table
  // ---------------------------------------------------------------------------
  enum class TTFlag { EXACT, LOWERBOUND, UPPERBOUND };
  struct TTEntry {
    uint64_t hash = 0;          // stored hash for collision verification
    int      value     = 0;
    int      depth     = 0;
    TTFlag   flag      = TTFlag::EXACT;
    std::string bestMove;
  };

  // ---------------------------------------------------------------------------
  // Piece-Square Tables (PST) — White's perspective.
  // Indexing: [rank][file], rank 0 = White's back rank (rank 1), rank 7 = promotion rank.
  // For Black pieces apply pst[7 - rank][file].
  // ---------------------------------------------------------------------------

  // Pawns: reward central and advanced positions.
  static const int PST_PAWN[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0 },  // rank 1 — pawns never start here
    {  5, 10, 10,-20,-20, 10, 10,  5 },
    {  5, -5,-10,  0,  0,-10, -5,  5 },
    {  0,  0,  0, 20, 20,  0,  0,  0 },
    {  5,  5, 10, 25, 25, 10,  5,  5 },
    { 10, 10, 20, 30, 30, 20, 10, 10 },
    { 50, 50, 50, 50, 50, 50, 50, 50 },  // near promotion
    {  0,  0,  0,  0,  0,  0,  0,  0 }   // promotion rank
  };

  // Knights: prefer central squares, penalise rim.
  static const int PST_KNIGHT[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50 },
    {-40,-20,  0,  5,  5,  0,-20,-40 },
    {-30,  5, 10, 15, 15, 10,  5,-30 },
    {-30,  0, 15, 20, 20, 15,  0,-30 },
    {-30,  5, 15, 20, 20, 15,  5,-30 },
    {-30,  0, 10, 15, 15, 10,  0,-30 },
    {-40,-20,  0,  0,  0,  0,-20,-40 },
    {-50,-40,-30,-30,-30,-30,-40,-50 }
  };

  // Bishops: prefer long diagonals.
  static const int PST_BISHOP[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20 },
    {-10,  5,  0,  0,  0,  0,  5,-10 },
    {-10, 10, 10, 10, 10, 10, 10,-10 },
    {-10,  0, 10, 10, 10, 10,  0,-10 },
    {-10,  5,  5, 10, 10,  5,  5,-10 },
    {-10,  0,  5, 10, 10,  5,  0,-10 },
    {-10,  0,  0,  0,  0,  0,  0,-10 },
    {-20,-10,-10,-10,-10,-10,-10,-20 }
  };

  // Rooks: prefer 7th rank and open files.
  static const int PST_ROOK[8][8] = {
    {  0,  0,  0,  5,  5,  0,  0,  0 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    {  5, 10, 10, 10, 10, 10, 10,  5 },  // 7th rank bonus
    {  0,  0,  0,  0,  0,  0,  0,  0 }
  };

  // Queens: avoid early development, prefer central activity.
  static const int PST_QUEEN[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20 },
    {-10,  0,  5,  0,  0,  0,  0,-10 },
    {-10,  5,  5,  5,  5,  5,  0,-10 },
    {  0,  0,  5,  5,  5,  5,  0, -5 },
    { -5,  0,  5,  5,  5,  5,  0, -5 },
    {-10,  0,  5,  5,  5,  5,  0,-10 },
    {-10,  0,  0,  0,  0,  0,  0,-10 },
    {-20,-10,-10, -5, -5,-10,-10,-20 }
  };

  // King middlegame: reward castled position (files a/b/g/h, back rank).
  static const int PST_KING_MID[8][8] = {
    { 20, 30, 10,  0,  0, 10, 30, 20 },  // back rank — castling squares good
    { 20, 20,  0,  0,  0,  0, 20, 20 },
    {-10,-20,-20,-20,-20,-20,-20,-10 },
    {-20,-30,-30,-40,-40,-30,-30,-20 },
    {-30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30 },
    {-30,-40,-40,-50,-50,-40,-40,-30 }
  };

  // King endgame: reward centralisation.
  static const int PST_KING_END[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50 },
    {-30,-20,-10,  0,  0,-10,-20,-30 },
    {-30,-10, 20, 30, 30, 20,-10,-30 },
    {-30,-10, 30, 40, 40, 30,-10,-30 },
    {-30,-10, 30, 40, 40, 30,-10,-30 },
    {-30,-10, 20, 30, 30, 20,-10,-30 },
    {-30,-30,  0,  0,  0,  0,-30,-30 },
    {-50,-30,-30,-30,-30,-30,-30,-50 }
  };

  // Look up PST bonus for a piece at (rank, file).
  int getPSTBonus(PieceType type, PlayerId owner, int rank, int file, bool endgame) {
    int r = (owner == PlayerId::WHITE) ? rank : (7 - rank);
    switch (type) {
      case PieceType::PAWN:   return PST_PAWN[r][file];
      case PieceType::KNIGHT: return PST_KNIGHT[r][file];
      case PieceType::BISHOP: return PST_BISHOP[r][file];
      case PieceType::ROOK:   return PST_ROOK[r][file];
      case PieceType::QUEEN:  return PST_QUEEN[r][file];
      case PieceType::KING:   return endgame ? PST_KING_END[r][file] : PST_KING_MID[r][file];
      default:                return 0;
    }
  }

  // ---------------------------------------------------------------------------
  // Zobrist-style hashing via FNV-1a (no pre-computed table needed).
  // Produces a uint64_t key covering pieces, square types/items, side to move,
  // turn count, and mana — same information as the old string key, much faster.
  // ---------------------------------------------------------------------------
  uint64_t computeZobristKey(Board& board, Player& active, Player& opponent, int turn_count) {
    constexpr uint64_t FNV1A_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t PRIME              = 1099511628211ULL;
    uint64_t h = FNV1A_OFFSET_BASIS;

    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        // Mix in square position so empty squares at different locations differ.
        h ^= static_cast<uint64_t>(r * 8 + c + 1);
        h *= PRIME;

        Square* sq = board.getSquare(c, r);
        if (!sq) continue;

        Piece* p = sq->getPiece();
        if (p) {
          for (unsigned char ch : p->getPieceId()) { h ^= ch; h *= PRIME; }
          h ^= static_cast<uint8_t>(p->getOwner() == PlayerId::WHITE ? 0x57u : 0x42u);
          h *= PRIME;
        }
        // Include square type and items so special-square state is hashed.
        for (unsigned char ch : sq->getType()) { h ^= ch; h *= PRIME; }
        h *= PRIME;
        if (sq->hasItem()) {
          for (unsigned char ch : sq->getItemId()) { h ^= ch; h *= PRIME; }
          h *= PRIME;
        }
      }
    }
    // Side to move, turn, and mana.
    h ^= static_cast<uint8_t>(active.getId() == PlayerId::WHITE ? 0x57u : 0x42u);
    h *= PRIME;
    h ^= static_cast<uint64_t>(turn_count);
    h *= PRIME;
    h ^= static_cast<uint64_t>(active.getMana() + opponent.getMana() * 31u);
    h *= PRIME;
    return h;
  }

  // ---------------------------------------------------------------------------
  // Move capture detection
  // ---------------------------------------------------------------------------
  bool moveIsCapture(const std::string& mv, Board& board, Player& active) {
    if (mv.find('x') != std::string::npos) return true;
    std::vector<std::string> tokens;
    Utils::tokenize(mv, tokens, ' ');
    if (tokens.empty()) return false;

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
    return false;
  }

  // ---------------------------------------------------------------------------
  // MVV/LVA (Most Valuable Victim / Least Valuable Attacker) scoring for captures.
  // Returns victim_value * 10 - attacker_value (higher = search first).
  // ---------------------------------------------------------------------------
  int simplePieceWeight(PieceType t) {
    switch (t) {
      case PieceType::PAWN:   return 1;
      case PieceType::KNIGHT: return 3;
      case PieceType::BISHOP: return 3;
      case PieceType::ROOK:   return 5;
      case PieceType::QUEEN:  return 9;
      case PieceType::KING:   return 20;
      default:                return 1;
    }
  }

  int getMvvLvaScore(const std::string& mv, Board& board, Player& /*active*/) {
    std::vector<std::string> tokens;
    Utils::tokenize(mv, tokens, ' ');

    // Locate source and destination squares around the 'x' token.
    std::string src_str, dst_str;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
      if (tokens[i] == "x" && i > 0 && i + 1 < tokens.size()) {
        src_str = tokens[i - 1];
        dst_str = tokens[i + 1];
        break;
      }
    }
    if (src_str.empty() || dst_str.empty()) return 0;
    if (!Coordinates::isValid(src_str) || !Coordinates::isValid(dst_str)) return 0;

    Coordinates src(src_str), dst(dst_str);
    Square* src_sq = board.getSquare(src.getFile(), src.getRank());
    Square* dst_sq = board.getSquare(dst.getFile(), dst.getRank());
    if (!src_sq || !dst_sq) return 0;

    Piece* attacker = src_sq->getPiece();
    Piece* victim   = dst_sq->getPiece();
    if (!attacker || !victim) return 0;

    return simplePieceWeight(victim->getType()) * 10 - simplePieceWeight(attacker->getType());
  }

  // ---------------------------------------------------------------------------
  // Opening book — returns a principle-based first move for turns 1–8,
  // or an empty string when outside that range.
  //
  // Parameters:
  //   active     – the player whose turn it is (WHITE or BLACK)
  //   turn_count – the current game turn (1-indexed)
  //   candidates – the full list of legal moves generated for this position
  //
  // Returns the first book entry found in `candidates`, or "" if no book move
  // is legal in the current position.  Verification against the legal-move
  // list is mandatory: special-piece configurations may make standard
  // square-notation moves illegal even on a typical starting board.
  // ---------------------------------------------------------------------------
  std::string getOpeningBookMove(Player& active, int turn_count,
                                  const std::vector<std::string>& candidates) {
    if (turn_count > 8) return "";

    std::vector<std::string> preferred;
    if (active.getId() == PlayerId::WHITE) {
      switch (turn_count) {
        case 1: preferred = {"e2 e4", "d2 d4", "c2 c4"};          break;
        case 3: preferred = {"g1 f3", "b1 c3", "f1 c4"};          break;
        case 5: preferred = {"f1 c4", "f1 e2", "c1 e3"};          break;
        default: break;
      }
    } else {
      switch (turn_count) {
        case 2: preferred = {"e7 e5", "d7 d5", "c7 c5", "e7 e6"}; break;
        case 4: preferred = {"g8 f6", "b8 c6", "f8 c5"};          break;
        case 6: preferred = {"f8 c5", "f8 e7"};                   break;
        default: break;
      }
    }

    for (const auto& m : preferred) {
      if (std::find(candidates.begin(), candidates.end(), m) != candidates.end())
        return m;
    }
    return "";
  }

} // namespace

SearchEngine::SearchEngine(Game& game, Board& board, int maxDepth, int timeLimitMs)
  : game_(game), board_(board), maxDepth_(maxDepth), timeLimitMs_(timeLimitMs) {}

std::string SearchEngine::findBestMove(Player& active, Player& opponent, int turn_count,
                                       bool frightened_king_cannot_capture,
                                       const std::set<std::string>& failed_commands)
{
  using clock = std::chrono::steady_clock;
  auto start_time = clock::now();
  auto end_time   = start_time + std::chrono::milliseconds(timeLimitMs_);

  // ---------------------------------------------------------------------------
  // Per-search state
  // ---------------------------------------------------------------------------
  // Transposition table keyed by 64-bit Zobrist hash.
  std::unordered_map<uint64_t, TTEntry> tt;
  tt.reserve(1u << 17);

  // 2-level killer move table: killer_moves[depth][0/1].
  std::vector<std::array<std::string, 2>> killer_moves(maxDepth_ + 2);

  // History heuristic: maps quiet move string → accumulated bonus.
  std::unordered_map<std::string, int> history_table;

  // Helper: clamp a depth value to a valid killer table index.
  auto killerIdx = [&](int d) -> int {
    return std::min(d, (int)killer_moves.size() - 1);
  };

  // ---------------------------------------------------------------------------
  // Helper: golden-pawn win check
  // ---------------------------------------------------------------------------
  auto checkGoldenPawnWin = [&](Player& currentPlayer) -> bool {
    int back_rank = (currentPlayer.getId() == PlayerId::WHITE) ? 7 : 0;
    for (int c = 0; c < 8; ++c) {
      Square* sq = board_.getSquare(c, back_rank);
      if (sq && sq->getPiece() &&
          sq->getPiece()->getOwner() == currentPlayer.getId() &&
          sq->getPiece()->getPieceId() == "PGLD")
        return true;
    }
    return false;
  };

  // ---------------------------------------------------------------------------
  // Helper: generate root candidate list (filtering previously failed commands).
  // ---------------------------------------------------------------------------
  auto generateCandidates = [&](Player& p) {
    std::vector<std::string> m = game_.generateAllLegalMoves(p, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> s = game_.generateAllLegalSpecials(p, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> cand;
    cand.reserve(m.size() + s.size());
    for (auto& it : m) if (!failed_commands.count(it)) cand.push_back(it);
    for (auto& it : s) if (!failed_commands.count(it)) cand.push_back(it);
    if (cand.empty()) cand.push_back("pass");
    return cand;
  };

  // ---------------------------------------------------------------------------
  // Static evaluation with PST, pawn structure, king safety, endgame bonuses.
  // ---------------------------------------------------------------------------
  auto evaluateStatic = [&](Player& a, Player& b) -> int {
    int score = 0;

    // Count total non-king material to detect endgame.
    int total_material = 0;
    for (int rf = 0; rf < 8; ++rf) {
      for (int cf = 0; cf < 8; ++cf) {
        Square* sq = board_.getSquare(cf, rf);
        if (!sq) continue;
        Piece* p = sq->getPiece();
        if (!p || p->getType() == PieceType::KING) continue;
        switch (p->getType()) {
          case PieceType::PAWN:   total_material += 100; break;
          case PieceType::KNIGHT: total_material += 320; break;
          case PieceType::BISHOP: total_material += 330; break;
          case PieceType::ROOK:   total_material += 500; break;
          case PieceType::QUEEN:  total_material += 900; break;
          default:                total_material += 100; break;
        }
      }
    }
    bool endgame = (total_material < 1800);

    // Track pawn file occupancy for structure evaluation.
    int a_pawns_on_file[8] = {};
    int b_pawns_on_file[8] = {};
    for (int rf = 0; rf < 8; ++rf) {
      for (int cf = 0; cf < 8; ++cf) {
        Square* sq = board_.getSquare(cf, rf);
        if (!sq) continue;
        Piece* p = sq->getPiece();
        if (!p || p->getType() != PieceType::PAWN) continue;
        if (p->getOwner() == a.getId()) a_pawns_on_file[cf]++;
        else                            b_pawns_on_file[cf]++;
      }
    }

    // Main material + PST loop.
    for (int rf = 0; rf < 8; ++rf) {
      for (int cf = 0; cf < 8; ++cf) {
        Square* sq = board_.getSquare(cf, rf);
        if (!sq) continue;
        Piece* p = sq->getPiece();
        if (!p) continue;

        int val = 0;
        switch (p->getType()) {
          case PieceType::PAWN:   val = 100;   break;
          case PieceType::KNIGHT: val = 320;   break;
          case PieceType::BISHOP: val = 330;   break;
          case PieceType::ROOK:   val = 500;   break;
          case PieceType::QUEEN:  val = 900;   break;
          case PieceType::KING:   val = 20000; break;
          default:                val = 100;   break;
        }
        // Special-piece bonuses.
        const std::string& pid = p->getPieceId();
        if      (pid == "PGLD") val += 300;
        else if (pid == "PEXP") val += 150;
        else if (pid == "RINV") val += 150;

        // PST positional bonus.
        val += getPSTBonus(p->getType(), p->getOwner(), rf, cf, endgame);

        bool is_a = (p->getOwner() == a.getId());
        int  sign = is_a ? 1 : -1;

        // Pawn structure evaluation.
        if (p->getType() == PieceType::PAWN) {
          int* own_pf = is_a ? a_pawns_on_file : b_pawns_on_file;
          int* opp_pf = is_a ? b_pawns_on_file : a_pawns_on_file;

          // Doubled pawn penalty.
          if (own_pf[cf] > 1) val -= 15;

          // Isolated pawn penalty.
          bool has_neighbour = (cf > 0 && own_pf[cf - 1] > 0) ||
                               (cf < 7 && own_pf[cf + 1] > 0);
          if (!has_neighbour) val -= 20;

          // Passed pawn: no opposing pawn on same or adjacent files in front.
          bool is_passed = (opp_pf[cf] == 0) &&
                           (cf == 0 || opp_pf[cf - 1] == 0) &&
                           (cf == 7 || opp_pf[cf + 1] == 0);
          if (is_passed) {
            int rank_adv = is_a ? rf : (7 - rf);  // distance advanced from back rank
            int pp_bonus = endgame ? (20 + rank_adv * 15) : (10 + rank_adv * 8);
            val += pp_bonus;
          }
        }

        score += sign * val;
      }
    }

    // King safety: count opponent pieces within a 5×5 area of each king.
    auto kingSafetyPenalty = [&](Player& defending, Player& attacking) -> int {
      King* k = board_.getKing(defending.getId());
      if (!k) return 0;
      int kf = k->getCoordinates().getFile();
      int kr = k->getCoordinates().getRank();
      int threats = 0;
      for (int dr = -2; dr <= 2; ++dr) {
        for (int dc = -2; dc <= 2; ++dc) {
          int nr = kr + dr, nc = kf + dc;
          if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) continue;
          Square* sq = board_.getSquare(nc, nr);
          if (!sq) continue;
          Piece* p = sq->getPiece();
          if (p && p->getOwner() == attacking.getId())
            threats += simplePieceWeight(p->getType());
        }
      }
      return threats * 3;
    };

    score -= kingSafetyPenalty(a, b);  // threats to a's king are bad for a
    score += kingSafetyPenalty(b, a);  // threats to b's king are good for a

    // Mobility: more legal moves is better.
    auto act_moves = game_.generateAllLegalMoves(a, turn_count, false, nullptr);
    auto opp_moves = game_.generateAllLegalMoves(b, turn_count, false, nullptr);
    score += static_cast<int>(act_moves.size()) * 5;
    score -= static_cast<int>(opp_moves.size()) * 5;

    // Mana advantage.
    score += a.getMana() * 3;
    score -= b.getMana() * 3;

    return score;
  };

  // ---------------------------------------------------------------------------
  // Quiescence search: extend captures to reduce horizon effect.
  // ---------------------------------------------------------------------------
  std::function<int(int,int,Player&,Player&)> quiescence;
  quiescence = [&](int alpha, int beta, Player& side, Player& other) -> int {
    if (clock::now() > end_time) return 0;

    int stand_pat = evaluateStatic(side, other);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    std::vector<std::string> caps;
    auto moves   = game_.generateAllLegalMoves(side, turn_count, false, nullptr);
    auto specials = game_.generateAllLegalSpecials(side, turn_count, false, nullptr);
    for (auto& m : moves)    if (m != "pass" && moveIsCapture(m, board_, side))    caps.push_back(m);
    for (auto& s : specials) if (s != "pass" && moveIsCapture(s, board_, side))    caps.push_back(s);

    // Order captures by MVV/LVA.
    std::sort(caps.begin(), caps.end(), [&](const std::string& a, const std::string& b) {
      return getMvvLvaScore(a, board_, side) > getMvvLvaScore(b, board_, side);
    });

    for (auto& mv : caps) {
      if (clock::now() > end_time) break;
      Board::UndoRecord rec = board_.makeMoveSimulation(mv, side, other, turn_count, frightened_king_cannot_capture);
      if (!rec.valid) continue;

      King* oppKing = board_.getKing(other.getId());
      bool pgld_win = (mv.find("PGLD") != std::string::npos) && checkGoldenPawnWin(side);
      int score = (!oppKing || pgld_win) ? 100000 : -quiescence(-beta, -alpha, other, side);
      board_.undoMoveSimulation(rec, side, other);

      if (score >= beta) return beta;
      if (score > alpha) alpha = score;
    }
    return alpha;
  };

  // ---------------------------------------------------------------------------
  // Negamax with:
  //  • Zobrist TT with collision verification
  //  • Check extension (+1 ply when side to move is in check)
  //  • Null move pruning (depth >= 3, not in check, R = 2)
  //  • 2-level killer moves + history heuristic + MVV/LVA move ordering
  //  • Late Move Reduction (depth >= 3, quiet non-killer moves, move_count >= 3)
  //  • Fixed TT flag convention (saves original_alpha)
  // ---------------------------------------------------------------------------
  std::function<int(int,int,int,Player&,Player&,bool)> negamax;
  negamax = [&](int depth, int alpha, int beta,
                Player& side, Player& other, bool allow_null) -> int
  {
    if (clock::now() > end_time) return 0;

    // --- Check extension ---
    King* my_king = board_.getKing(side.getId());
    bool in_check = my_king && my_king->inCheck(board_, side, turn_count);
    int  ext       = (in_check && depth > 0) ? 1 : 0;
    int  actual_d  = depth + ext;

    if (actual_d == 0)
      return quiescence(alpha, beta, side, other);

    // --- TT probe ---
    int      original_alpha = alpha;
    uint64_t hash_key       = computeZobristKey(board_, side, other, turn_count);
    auto it = tt.find(hash_key);
    if (it != tt.end() && it->second.hash == hash_key && it->second.depth >= actual_d) {
      const TTEntry& ent = it->second;
      if (ent.flag == TTFlag::EXACT)                                return ent.value;
      if (ent.flag == TTFlag::LOWERBOUND) alpha = std::max(alpha, ent.value);
      else                                beta  = std::min(beta,  ent.value);
      if (alpha >= beta) return ent.value;
    }
    std::string tt_move = (it != tt.end() && it->second.hash == hash_key) ? it->second.bestMove : "";

    // --- Null move pruning ---
    if (allow_null && actual_d >= 3 && !in_check) {
      int R = 2;
      int null_score = -negamax(actual_d - 1 - R, -beta, -beta + 1, other, side, false);
      if (null_score >= beta) return beta;
    }

    // --- Generate moves ---
    std::vector<std::string> moves   = game_.generateAllLegalMoves(side, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> specials = game_.generateAllLegalSpecials(side, turn_count, frightened_king_cannot_capture, nullptr);
    std::vector<std::string> cand;
    cand.reserve(moves.size() + specials.size());
    cand.insert(cand.end(), moves.begin(), moves.end());
    cand.insert(cand.end(), specials.begin(), specials.end());

    if (cand.empty()) {
      // Checkmate or stalemate.
      return in_check ? (-90000 + (maxDepth_ - actual_d)) : 0;
    }

    // --- Move ordering: TT move > MVV/LVA captures > killers > history > evaluateMove ---
    const auto& killers = killer_moves[killerIdx(actual_d)];
    std::sort(cand.begin(), cand.end(), [&](const std::string& a, const std::string& b) {
      auto score_of = [&](const std::string& m) -> int {
        if (m == tt_move) return 2000000;
        if (moveIsCapture(m, board_, side))
          return 1000000 + getMvvLvaScore(m, board_, side);
        if (m == killers[0]) return 900000;
        if (m == killers[1]) return 800000;
        auto hi = history_table.find(m);
        int  h  = (hi != history_table.end()) ? hi->second : 0;
        return h + game_.evaluateMove(m, side.getId());
      };
      return score_of(a) > score_of(b);
    });

    int         bestValue     = std::numeric_limits<int>::min() / 4;
    std::string bestMoveLocal;
    int         moves_searched = 0;

    for (const auto& mv : cand) {
      if (clock::now() > end_time) break;

      // "pass" move: give the turn away (penalised slightly).
      if (mv == "pass") {
        int val = -negamax(actual_d - 1, -beta, -alpha, other, side, false) - 10;
        if (val > bestValue) { bestValue = val; bestMoveLocal = mv; }
        alpha = std::max(alpha, bestValue);
        if (alpha >= beta) break;
        ++moves_searched;
        continue;
      }

      Board::UndoRecord rec = board_.makeMoveSimulation(mv, side, other, turn_count, frightened_king_cannot_capture);
      if (!rec.valid) continue;

      King* oppKing = board_.getKing(other.getId());
      bool  pgld_win = (mv.find("PGLD") != std::string::npos) && checkGoldenPawnWin(side);
      bool  is_cap   = moveIsCapture(mv, board_, side);

      int score;
      if (!oppKing || pgld_win) {
        score = 100000;
      } else {
        int new_d = actual_d - 1;

        // Late Move Reduction: reduce depth for quiet moves searched late.
        bool use_lmr = !is_cap && !in_check && moves_searched >= 3 && actual_d >= 3 &&
                       mv != killers[0] && mv != killers[1] && mv != tt_move;
        if (use_lmr) {
          // Reduced-depth search with a null window.
          score = -negamax(new_d - 1, -alpha - 1, -alpha, other, side, true);
          // Re-search at full depth if score is interesting.
          if (score > alpha)
            score = -negamax(new_d, -beta, -alpha, other, side, true);
        } else {
          score = -negamax(new_d, -beta, -alpha, other, side, true);
        }
      }

      board_.undoMoveSimulation(rec, side, other);
      ++moves_searched;

      if (score > bestValue) {
        bestValue     = score;
        bestMoveLocal = mv;
        if (bestValue > alpha) {
          alpha = bestValue;
          if (!is_cap) {
            // Update 2-level killer table.
            int ki = killerIdx(actual_d);
            if (mv != killer_moves[ki][0]) {
              killer_moves[ki][1] = killer_moves[ki][0];
              killer_moves[ki][0] = mv;
            }
            // Update history heuristic.
            history_table[mv] += actual_d * actual_d;
          }
        }
      }
      if (alpha >= beta) break;  // beta cutoff
    }

    // --- TT store (fixed flag logic using original_alpha) ---
    TTEntry entry;
    entry.hash     = hash_key;
    entry.value    = bestValue;
    entry.depth    = actual_d;
    entry.bestMove = bestMoveLocal;
    if      (bestValue <= original_alpha) entry.flag = TTFlag::UPPERBOUND;
    else if (bestValue >= beta)           entry.flag = TTFlag::LOWERBOUND;
    else                                  entry.flag = TTFlag::EXACT;
    tt[hash_key] = std::move(entry);

    return bestValue;
  };

  // ---------------------------------------------------------------------------
  // Iterative deepening with aspiration windows and improved time management.
  // ---------------------------------------------------------------------------
  std::vector<std::string> root_candidates = generateCandidates(active);

  // Check opening book before any search.
  std::string book_move = getOpeningBookMove(active, turn_count, root_candidates);
  if (!book_move.empty()) return book_move;

  // Initial sort by evaluateMove heuristic.
  std::sort(root_candidates.begin(), root_candidates.end(), [&](const std::string& a, const std::string& b) {
    return game_.evaluateMove(a, active.getId()) > game_.evaluateMove(b, active.getId());
  });

  std::string best_move  = root_candidates.front();
  int         best_score = std::numeric_limits<int>::min();

  // Track time consumed per depth for branching-factor estimation.
  std::vector<int64_t> depth_times;
  // Bound on alpha/beta used in place of ±∞ to avoid signed-overflow.
  constexpr int SEARCH_INFINITY = std::numeric_limits<int>::max() / 4;
  // Conservative estimate of the per-ply branching factor used in the time
  // prediction heuristic.  Real alpha-beta typically achieves ~sqrt(b) after
  // ordering, but 4 is used here to avoid starting a depth that cannot finish.
  constexpr int64_t BRANCHING_FACTOR_ESTIMATE = 4;

  // Lambda: search all root moves with the given alpha/beta window.
  // Updates best_score_out and best_move_out.
  auto searchRoot = [&](int depth, int alpha, int beta,
                         int& best_score_out, std::string& best_move_out) {
    // Re-order using the TT best move from the previous iteration.
    uint64_t rootKey = computeZobristKey(board_, active, opponent, turn_count);
    auto rt = tt.find(rootKey);
    std::string rtbest = (rt != tt.end() && rt->second.hash == rootKey) ? rt->second.bestMove : "";
    if (!rtbest.empty()) {
      auto pos = std::find(root_candidates.begin(), root_candidates.end(), rtbest);
      if (pos != root_candidates.end()) std::iter_swap(root_candidates.begin(), pos);
    }

    for (const auto& mv : root_candidates) {
      if (clock::now() > end_time) break;

      if (mv == "pass") {
        int val = -negamax(depth - 1, -SEARCH_INFINITY, SEARCH_INFINITY, opponent, active, false) - 10;
        if (val > best_score_out) { best_score_out = val; best_move_out = mv; }
        continue;
      }

      Board::UndoRecord rec = board_.makeMoveSimulation(mv, active, opponent, turn_count, frightened_king_cannot_capture);
      if (!rec.valid) continue;

      King* oppKing = board_.getKing(opponent.getId());
      bool  pgld_win = (mv.find("PGLD") != std::string::npos) && checkGoldenPawnWin(active);
      int   val;
      if (!oppKing || pgld_win) {
        val = 100000;
      } else {
        val = -negamax(depth - 1, -beta, -alpha, opponent, active, true);
      }
      board_.undoMoveSimulation(rec, active, opponent);

      if (val > best_score_out) {
        best_score_out = val;
        best_move_out  = mv;
      }
      if (val > alpha) alpha = val;  // tighten lower bound for subsequent moves
    }
  };

  for (int depth = 1; depth <= maxDepth_; ++depth) {
    if (clock::now() > end_time) break;

    // Better time management: estimate next-depth time using branching factor ~4.
    if (depth > 2 && !depth_times.empty()) {
      int64_t elapsed   = std::chrono::duration_cast<std::chrono::milliseconds>(
                            clock::now() - start_time).count();
      int64_t remaining = static_cast<int64_t>(timeLimitMs_) - elapsed;
      int64_t predicted = depth_times.back() * BRANCHING_FACTOR_ESTIMATE;
      if (predicted > remaining) break;
    }

    auto   depth_start    = clock::now();
    int    best_score_this = std::numeric_limits<int>::min();
    std::string best_move_this = best_move;

    // --- Aspiration windows ---
    bool   valid_prev = (best_score > std::numeric_limits<int>::min() / 2);
    int    asp_lo     = valid_prev ? (best_score - 50) : -SEARCH_INFINITY;
    int    asp_hi     = valid_prev ? (best_score + 50) :  SEARCH_INFINITY;

    searchRoot(depth, asp_lo, asp_hi, best_score_this, best_move_this);

    // If the result fell outside the window, re-search with a full window.
    if (clock::now() <= end_time &&
        (best_score_this <= asp_lo || best_score_this >= asp_hi) && valid_prev) {
      best_score_this = std::numeric_limits<int>::min();
      best_move_this  = best_move;
      searchRoot(depth, -SEARCH_INFINITY, SEARCH_INFINITY, best_score_this, best_move_this);
    }

    if (clock::now() > end_time) break;

    best_move  = best_move_this;
    best_score = best_score_this;

    int64_t depth_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         clock::now() - depth_start).count();
    depth_times.push_back(std::max(int64_t{1}, depth_ms));

    if (best_score >= 90000) break;  // forced win found — no need to search deeper
  }

  return best_move;
}
