//----------------------------------------------------------------------------------------------------------------------
/// The Command class parses user input into game commands and processes their execution, handling 
/// standard moves, special abilities, potion usage, and match logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Command.hpp"
#include "Utils.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include "Piece.hpp"
#include "Pawn.hpp"
#include "Rook.hpp"
#include "Knight.hpp"
#include "Bishop.hpp"
#include "Queen.hpp"
#include "King.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <iostream>
#include <map>
#include <set>
#include <sstream>



namespace CommandUtils
{
  std::string upperCopy(std::string value)
  {
    Utils::toUpperCase(value);
    return value;
  }

  std::string lowerCopy(std::string value)
  {
    Utils::toLowerCase(value);
    return value;
  }

  bool isPieceId(const std::string& piece_id)
  {
    static const std::set<std::string> ids = {
      "P", "PGLD", "PIPT", "PSTB", "PNRV", "PEXP",
      "R", "RINV", "RPNT",
      "N", "NJMP", "NICE",
      "B", "BCLR", "BPRC",
      "Q", "QFLP", "QJMP", "QHNGR",
      "K", "KFRT", "KARC"
    };
    return ids.find(piece_id) != ids.end();
  }

  bool isSpecialPiece(const std::string& piece_id)
  {
    static const std::set<std::string> ids = {
      "PIPT", "PSTB", "PNRV", "PEXP", "RINV", "RPNT",
      "BCLR", "BPRC", "QFLP", "QJMP", "KARC"
    };
    return ids.find(piece_id) != ids.end();
  }

  int expectedSpecialParameterCount(const std::string& piece_id)
  {
    static const std::map<std::string, int> counts = {
      {"PIPT", 2}, {"PSTB", 1}, {"PNRV", 1}, {"PEXP", 2},
      {"RINV", 2}, {"RPNT", 2}, {"BCLR", 2}, {"BPRC", 2},
      {"QFLP", 3}, {"QJMP", 2}, {"KARC", 2}
    };
    auto found = counts.find(piece_id);
    return found == counts.end() ? -1 : found->second;
  }

  bool isValidPieceTypeParameter(const std::string& value)
  {
    std::string upper = upperCopy(value);
    return upper == "R" || upper == "N" || upper == "B" || upper == "Q";
  }

  bool isValidTurnCount(const std::string& value)
  {
    if(value.empty()) return false;
    for(char c : value)
    {
      if(!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    try
    {
      long long number = std::stoll(value);
      return number >= 1 && number <= 65537;
    }
    catch(...)
    {
      return false;
    }
  }

  int specialManaCost(const std::string& piece_id)
  {
    static const std::map<std::string, int> costs = {
      {"PSTB", 5}, {"PNRV", 1}, {"PEXP", 3}, {"BCLR", 3}, {"QJMP", 2}
    };
    auto found = costs.find(piece_id);
    if(found == costs.end()) return 0; 
    return found->second;
  }

  int variableManaCost(const std::string& piece_id, const Coordinates& coordinates, const std::vector<std::string>& parameters,
                      Board& board, Player& active_player)
  {
    if(piece_id == "PIPT")
    {
      int back_rank = (active_player.getId() == PlayerId::WHITE) ? 7 : 0;
      int rank_difference = std::abs(back_rank - coordinates.getRank());
      std::string promotion_type = upperCopy(parameters.at(1));
      static const std::map<std::string, int> piece_values = {
        {"R", 5}, {"N", 3}, {"B", 3}, {"Q", 9}
      };
      return rank_difference * piece_values.at(promotion_type);
    }
    else if (piece_id == "RINV") return std::stoi(parameters.at(1));
    else if (piece_id == "KARC")
    {
      Coordinates target(parameters.at(1));
      return std::max(std::abs(target.getFile() - coordinates.getFile()),
                      std::abs(target.getRank() - coordinates.getRank()));
    }
    else if (piece_id == "RPNT")
    {
      Coordinates target_coordinates = Coordinates(parameters.at(1));
      int source_file = coordinates.getFile();
      int source_rank = coordinates.getRank();
      int target_file = target_coordinates.getFile();
      int target_rank = target_coordinates.getRank();
      return std::max(std::abs(target_file - source_file), std::abs(target_rank - source_rank));
    }
    else if(piece_id == "BPRC")
    {
      Coordinates target(parameters.at(1));
      Square* target_square = board.getSquare(target.getFile(), target.getRank());
      Piece* target_piece = target_square ? target_square->getPiece() : nullptr;
      return 3 * target_piece->getValue();
    }
    else if(piece_id == "QFLP")
    {
      Coordinates bounce(parameters.at(1));
      Coordinates target(parameters.at(2));
      return std::max(std::abs(target.getFile() - bounce.getFile()),
                      std::abs(target.getRank() - bounce.getRank()));
    }
    return 0;
  }

  bool isPotionId(const std::string& item_id)
  {
    return item_id == "FREEZE" || item_id == "TP" || item_id == "EVENODD" || item_id == "LUKE";
  }

  int expectedUseParameterCount(const std::string& item_id)
  {
    if(item_id == "FREEZE") return 2;
    if(item_id == "TP") return 2;
    if(item_id == "EVENODD") return 3;
    if(item_id == "LUKE") return 1;
    return -1;
  }

  bool validParity(const std::string& value)
  {
    std::string lower = lowerCopy(value);
    return lower == "even" || lower == "odd";
  }

  bool isTeleportTargetLegal(Board& board, PlayerId owner, int target_rank, int target_file)
  {
    if(board.getSquare(target_file, target_rank) == nullptr || !board.getSquare(target_file, target_rank)->isEmpty()) return false;

    if(owner == PlayerId::WHITE)
    {
      int farthest_rank = -1;
      for(int r = 0; r < 8; ++r)
      {
        for(int c = 0; c < 8; ++c)
        {
          Square* square = board.getSquare(c, r);
          Piece* piece = square ? square->getPiece() : nullptr;
          if(piece != nullptr && piece->getOwner() == owner) farthest_rank = std::max(farthest_rank, r);
        }
      }
      return target_rank < farthest_rank;
    }

    int farthest_rank = 8;
    for(int r = 0; r < 8; ++r)
    {
      for(int c = 0; c < 8; ++c)
      {
        Square* square = board.getSquare(c, r);
        Piece* piece = square ? square->getPiece() : nullptr;
        if(piece != nullptr && piece->getOwner() == owner) farthest_rank = std::min(farthest_rank, r);
      }
    }
    return target_rank > farthest_rank;
  }

  bool executeSkywalker(Board& board, Piece* piece)
  {
    const int file = piece->getCoordinates().getFile();
    const int rank = piece->getCoordinates().getRank();
    const int forward = piece->getOwner() == PlayerId::WHITE ? 1 : -1;
    const int first_rank = rank + forward;
    if(board.getSquare(file, first_rank) == nullptr || board.getSquare(file, first_rank)->getPiece() == nullptr) return false;

    int last_rank = first_rank;
    while(board.getSquare(file, last_rank + forward) != nullptr &&
          board.getSquare(file, last_rank + forward)->getPiece() != nullptr)
    {
      last_rank += forward;
    }

    if(board.getSquare(file, last_rank + forward) == nullptr) return false;

    for(int current_rank = last_rank; current_rank != rank; current_rank -= forward)
    {
      Square* source = board.getSquare(file, current_rank);
      Square* target = board.getSquare(file, current_rank + forward);
      if(source == nullptr || target == nullptr) return false;
      std::unique_ptr<Piece> pushed = source->releasePiece();
      if(pushed) pushed->setCoordinates(Coordinates(file, current_rank + forward));
      target->setPiece(std::move(pushed));
    }
    return true;
  }

  void printPrisonForPlayer(CommandLine& command_line, Player& player)
  {
    std::map<std::string, std::pair<int, int>> counts;
    for(const auto& piece : player.getPrison())
    {
      if(piece)
      {
        auto& entry = counts[piece->getPieceId()];
        entry.first++;
        entry.second = piece->getValue();
      }
    }

    std::vector<std::pair<std::string, std::pair<int, int>>> entries(counts.begin(), counts.end());
    std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs)
    {
      if(lhs.second.second != rhs.second.second) return lhs.second.second > rhs.second.second;
      return lhs.first < rhs.first;
    });

    command_line.printMessage("D_BORDER_PRISON");
    std::cout << (player.getId() == PlayerId::WHITE ? "White" : "Black") << ":\n";

    for(std::size_t index = 0; index < entries.size(); ++index)
    {
      if(index != 0) std::cout << ", ";
      if(entries[index].second.first > 1)
      {
        std::cout << entries[index].second.first << "x";
      }
      std::cout << entries[index].first;
    }
    std::cout << "\n";
    command_line.printMessage("D_BORDER_D");
  }
}

using namespace CommandUtils;

Command::Command(std::vector<std::string> &input_tokens)
{
  if(input_tokens.empty()) 
  {
    type_ = CommandType::UNKNOWN;
    return;
  }

  std::string command_word = input_tokens.at(0);
  Utils::toLowerCase(command_word);

  if(command_word == "quit")        type_ = CommandType::QUIT;
  else if(command_word == "board")  type_ = CommandType::BOARD;
  else if(command_word == "help")   type_ = CommandType::HELP;
  else if(command_word == "info")   type_ = CommandType::INFO;
  else if(command_word == "prison") type_ = CommandType::PRISON;
  else if(command_word == "special") type_ = CommandType::SPECIAL;
  else if(command_word == "move")   type_ = CommandType::MOVE;
  else if(command_word == "use")    type_ = CommandType::USE;
  else if(command_word == "pass")   type_ = CommandType::PASS;
  else if(command_word == "resign") type_ = CommandType::RESIGN;
  else if(command_word == "draw")   type_ = CommandType::DRAW;
  else if(command_word == "history") type_ = CommandType::HISTORY;
  else if(command_word == "whoami")  type_ = CommandType::WHOAMI;
  else if(command_word == "play")    type_ = CommandType::PLAY;
  else                               type_ = CommandType::UNKNOWN;

  for(std::size_t i = 1; i < input_tokens.size(); ++i) 
  {
    parameters_.push_back(input_tokens.at(i));
  }
}

bool Command::executeCommand(CommandLine& command_line, bool& turn_ended, Board& board, Player& active_player,
                             const Coordinates* forced_start, int turn_count,
                             bool frightened_king_cannot_capture)
{
  if(type_ == CommandType::HELP)
  {
    std::cout << "=== Commands ============================================================================\n";
    std::cout << "- help\n";
    std::cout << "    Prints this help text.\n";
    std::cout << "\n";
    std::cout << "- quit\n";
    std::cout << "    Terminates the game.\n";
    std::cout << "\n";
    std::cout << "- board\n";
    std::cout << "    Toggles the board printing.\n";
    std::cout << "\n";
    std::cout << "- info <PIECE_ID>\n";
    std::cout << "    Prints piece information.\n";
    std::cout << "    <PIECE_ID>: The piece ID to be explained.\n";
    std::cout << "\n";
    std::cout << "- history\n";
    std::cout << "    Prints the move history in modified chess notation.\n";
    std::cout << "\n";
    std::cout << "- prison <PLAYER_ID>\n";
    std::cout << "    Lists pieces captured by the specified player.\n";
    std::cout << "    <PLAYER_ID>: [White/Black]\n";
    std::cout << "\n";
    std::cout << "- pass\n";
    std::cout << "    Ends the current player's turn after a move or special ability.\n";
    std::cout << "\n";
    std::cout << "- draw\n";
    std::cout << "    Offers a draw to the opponent.\n";
    std::cout << "\n";
    std::cout << "- resign\n";
    std::cout << "    Resigns the game (loss).\n";
    std::cout << "\n";
    std::cout << "- move <MOVE>\n";
    std::cout << "    Moves a piece using simplified chess notation.\n";
    std::cout << "    <MOVE>: a move in the simplified chess notation format.\n";
    std::cout << "\n";
    std::cout << "- use <SQUARE> [...]\n";
    std::cout << "    Uses a potion.\n";
    std::cout << "    <SQUARE>: The location of the piece, whose potion will be used.\n";
    std::cout << "    [...]: Variable amount of parameters depending on the potion.\n";
    std::cout << "\n";
    std::cout << "- special <SQUARE> [...]\n";
    std::cout << "    Activates a piece's special ability.\n";
    std::cout << "    <SQUARE>: The square where the special piece is located.\n";
    std::cout << "    [...]: Variable parameters depending on the piece (use info for a piece by piece description).\n";
    std::cout << "\n";
    std::cout << "=========================================================================================\n";
    turn_ended = false;
    return true;
  }

  if(type_ == CommandType::INFO)
  {
    std::string piece_id = upperCopy(parameters_.at(0));
    if(!isPieceId(piece_id))
    {
      command_line.printMessage("E_INV_PARAM_PIECE");
      turn_ended = false;
      return false;
    }

    struct InfoData
    {
      std::string short_name;
      std::string mana;
      bool has_special;
    };
    static const std::map<std::string, InfoData> info_map = 
    {
      { "P",     { "♟p",  "",   false } },
      { "PGLD",  { "♟pg", "",   false } },
      { "PIPT",  { "♟pi", "XX", true  } },
      { "PSTB",  { "♟p+", "05", true  } },
      { "PNRV",  { "♟p-", "01", true  } },
      { "PEXP",  { "♟p!", "03", true  } },

      { "R",     { "♜r",  "",   false } },
      { "RINV",  { "♜ri", "XX", true  } },
      { "RPNT",  { "♜rp", "XX", true  } },

      { "N",     { "♞n",  "",   false } },
      { "NJMP",  { "♞nj", "",   false } },
      { "NICE",  { "♞ni", "",   false } },

      { "B",     { "♝b",  "",   false } },
      { "BCLR",  { "♝bc", "03", true  } },
      { "BPRC",  { "♝bp", "XX", true  } },

      { "Q",     { "♛q",  "",   false } },
      { "QFLP",  { "♛qf", "XX", true  } },
      { "QJMP",  { "♛qj", "02", true  } },
      { "QHNGR", { "♛qh", "",   false } },

      { "K",     { "♚k",  "",   false } },
      { "KFRT",  { "♚kf", "",   false } },
      { "KARC",  { "♚ka", "XX", true  } }
    };

    const InfoData& data = info_map.at(piece_id);
    command_line.printMessage("D_BORDER_INFO_B");
    std::cout << "[" << piece_id << " | " << data.short_name << "] ";
    command_line.printMessage("D_N_" + piece_id);
    if(!data.mana.empty())
    {
      std::cout << "Mana: " << data.mana << "\n";
    }
    std::cout << "Description: ";
    command_line.printMessage("D_I_" + piece_id);
    std::cout << "Special: ";
    if(data.has_special)
    {
      command_line.printMessage("D_S_" + piece_id);
    }
    else
    {
      std::cout << "None\n";
    }
    command_line.printMessage("D_BORDER_INFO_E");
    turn_ended = false;
    return true;
  }

  if(type_ == CommandType::PRISON)
  {
    std::string player = lowerCopy(parameters_.at(0));
    if(player != "white" && player != "black")
    {
      command_line.printMessage("E_INV_PARAM_PLAYER");
      turn_ended = false;
      return false;
    }

    
    printPrisonForPlayer(command_line, active_player);
    turn_ended = false;
    return true;
  }

  if(type_ == CommandType::SPECIAL)
  {
    std::string square_str = parameters_.at(0);
    if(!Coordinates::isValid(square_str))
    {
      command_line.printMessage("E_INV_PARAM_SQUARE");
      turn_ended = false;
      return false;
    }

    Coordinates coordinates(square_str);
    Square* square = board.getSquare(coordinates.getFile(), coordinates.getRank());
    Piece* piece = square ? square->getPiece() : nullptr;
    if(piece == nullptr || piece->getOwner() != active_player.getId())
    {
      command_line.printMessage("E_PLAYER_PIECE_NOT_FOUND");
      turn_ended = false;
      return false;
    }

    std::string piece_id = piece->getPieceId();
    if(!isSpecialPiece(piece_id))
    {
      command_line.printMessage("E_NO_SPECIAL_POWER");
      turn_ended = false;
      return false;
    }

    int expected_count = expectedSpecialParameterCount(piece_id);
    if(expected_count < 0 || static_cast<int>(parameters_.size()) != expected_count)
    {
      command_line.printMessage("E_INVALID_PARAM_COUNT_SPECIAL");
      turn_ended = false;
      return false;
    }

    if(piece->getFrozen())
    {
      command_line.printMessage("E_PIECE_FROZEN");
      turn_ended = false;
      return false;
    }

    if(piece_id == "PIPT")
    {
      if(!isValidPieceTypeParameter(parameters_.at(1)))
      {
        command_line.printMessage("E_INV_PARAM_PIECE_TYPE");
        turn_ended = false;
        return false;
      }
    }
    else if(piece_id == "RINV")
    {
      if(!isValidTurnCount(parameters_.at(1)))
      {
        command_line.printMessage("E_INV_PARAM_TURN_COUNT");
        turn_ended = false;
        return false;
      }
    }
    else
    {
      for(std::size_t index = 1; index < parameters_.size(); ++index)
      {
        if(!Coordinates::isValid(parameters_.at(index)))
        {
          command_line.printMessage("E_INV_PARAM_SPECIAL_SQUARE");
          turn_ended = false;
          return false;
        }
      }
    }

    int mana_cost = specialManaCost(piece_id);
    if(mana_cost == 0)
    {
      mana_cost = variableManaCost(piece_id, coordinates, parameters_, board, active_player);
    }
    if(active_player.getMana() < mana_cost)
    {
      command_line.printMessage("E_INSUFFICIENT_MANA");
      turn_ended = false;
      return false;
    }
    std::string return_value = piece->special(&active_player, coordinates.getFile(), coordinates.getRank(), parameters_, &board);
    if(return_value != "")
    {
      command_line.printMessage(return_value);
      turn_ended = false;
      return false;
    }
    active_player.setMana(active_player.getMana() - mana_cost);
    board.clearEnPassant();
    turn_ended = true;
    return true;
  }

  if(type_ == CommandType::MOVE)
  {
    King* active_king = board.getKing(active_player.getId());
    if(active_king != nullptr && active_king->inCheckmate(board, active_player, turn_count,
                                                          frightened_king_cannot_capture))
    {
      command_line.printMessage("E_INVALID_MOVE");
      turn_ended = false;
      return false;
    }

    Move parsed_move(parameters_.at(0));
    if(!parsed_move.isValid())
    {
      command_line.printMessage("E_INV_PARAM_MOVE");
      turn_ended = false;
      return false;
    }

    Coordinates target_coordinates(parsed_move.getTargetSquare());
    int target_file = target_coordinates.getFile();
    int target_rank = target_coordinates.getRank();

    auto coordinateString = [](const Coordinates& coordinates) -> std::string
    {
      std::string result;
      result += static_cast<char>('a' + coordinates.getFile());
      result += static_cast<char>('1' + coordinates.getRank());
      return result;
    };

    auto candidate_is_legal = [&](Piece* p, int r, int c, PieceType requested_type,
                                  char requested_start_file, char requested_promotion_type,
                                  bool requested_capture) -> bool
    {
      if(p == nullptr || p->getOwner() != active_player.getId() || p->getType() != requested_type)
      {
        return false;
      }
      if(forced_start != nullptr && (forced_start->getFile() != c || forced_start->getRank() != r))
      {
        return false;
      }
      if(requested_start_file != '\0' && c != (requested_start_file - 'a'))
      {
        return false;
      }
      if(!p->canMoveOnTurn(turn_count))
      {
        return false;
      }
      if(frightened_king_cannot_capture && p == active_king &&
         p->getPieceId() == "KFRT" && requested_capture)
      {
        return false;
      }

      const bool moving_pawn = p->getType() == PieceType::PAWN;
      const bool reaches_back_rank = moving_pawn &&
                                     target_rank == (p->getOwner() == PlayerId::WHITE ? 7 : 0);
      if(moving_pawn)
      {
        if(reaches_back_rank && requested_promotion_type == '\0')
        {
          return false;
        }
        if(!reaches_back_rank && requested_promotion_type != '\0')
        {
          return false;
        }
      }

      if(!p->move(r, c, target_rank, target_file, requested_capture, board, active_player, true))
      {
        return false;
      }

      if(active_king != nullptr && active_king->wouldBeInCheckAfterMove(board, active_player, p, r, c,
                                                                        target_rank, target_file,
                                                                        requested_capture, turn_count))
      {
        return false;
      }
      return true;
    };

    auto collect_candidates = [&](PieceType requested_type, char requested_start_file,
                                  char requested_promotion_type, bool requested_capture)
    {
      std::vector<Coordinates> candidates;
      for(int r = 0; r < 8; ++r)
      {
        for(int c = 0; c < 8; ++c)
        {
          Square* sq = board.getSquare(c, r);
          Piece* p = sq ? sq->getPiece() : nullptr;
          if(candidate_is_legal(p, r, c, requested_type, requested_start_file,
                                requested_promotion_type, requested_capture))
          {
            candidates.emplace_back(c, r);
          }
        }
      }
      return candidates;
    };

    auto execute_from = [&](const Coordinates& start_coordinates, PieceType requested_type,
                            char requested_start_file, char requested_promotion_type,
                            bool requested_capture) -> bool
    {
      int r = start_coordinates.getRank();
      int c = start_coordinates.getFile();
      Square* sq = board.getSquare(c, r);
      Piece* p = sq ? sq->getPiece() : nullptr;
      if(!candidate_is_legal(p, r, c, requested_type, requested_start_file,
                             requested_promotion_type, requested_capture))
      {
        return false;
      }

      const bool moving_pawn = p->getType() == PieceType::PAWN;
      const bool reaches_back_rank = moving_pawn &&
                                     target_rank == (p->getOwner() == PlayerId::WHITE ? 7 : 0);
      const std::string moving_piece_id = p->getPieceId();
      const bool double_pawn_step = moving_pawn && !requested_capture && std::abs(target_rank - r) == 2;

      if(!p->move(r, c, target_rank, target_file, requested_capture, board, active_player))
      {
        return false;
      }

      if(double_pawn_step)
      {
        board.setEnPassantPawn(target_file, target_rank, active_player.getId());
      }
      else
      {
        board.clearEnPassant();
      }

      Square* target_square = board.getSquare(target_file, target_rank);
      Piece* target_piece = target_square ? target_square->getPiece() : nullptr;
      if(reaches_back_rank && moving_piece_id != "PGLD" && target_piece != nullptr &&
         target_piece->getOwner() == active_player.getId() && target_piece->getType() == PieceType::PAWN)
      {
        Pawn* pawn = dynamic_cast<Pawn*>(target_piece);
        if(pawn != nullptr)
        {
          std::string promotion_type(1, requested_promotion_type);
          pawn->promotion(promotion_type, board, target_file, target_rank);
        }
      }

      if(target_square && target_square->getType() == "BOOST")
      {
        turn_ended = false;
      }
      else
      {
        turn_ended = true;
      }
      return true;
    };

    bool ambiguous_input_handled = false;

    auto resolve_and_execute = [&](PieceType requested_type, char requested_start_file,
                                   char requested_promotion_type, bool requested_capture) -> bool
    {
      std::vector<Coordinates> candidates = collect_candidates(requested_type, requested_start_file,
                                                               requested_promotion_type, requested_capture);
      if(candidates.empty())
      {
        return false;
      }

      Coordinates selected_start = candidates.front();
      bool was_ambiguous = false;

      if(forced_start == nullptr && candidates.size() > 1)
      {
        command_line.printMessage("D_AMBIGUOUS_MOVE");
        std::cout << '\n';
        command_line.printPrompt(active_player.getId());

        std::string answer;
        std::getline(std::cin, answer);

        if (answer == "play")
        {
          // choose first candidate
          answer = coordinateString(candidates.front()); 
          std::cout << "AI: " << answer << "\n";
        }

        if(std::cin.eof())
        {
          ambiguous_input_handled = true;
          turn_ended = false;
          return false;
        }
        Utils::trim(answer);
        std::string lower_answer = lowerCopy(answer);
        if(lower_answer == "cancel")
        {
          ambiguous_input_handled = true;
          turn_ended = false;
          return false;
        }
        if(!Coordinates::isValid(answer))
        {
          command_line.printMessage("E_INV_PARAM_SQUARE");
          ambiguous_input_handled = true;
          turn_ended = false;
          return false;
        }
        Coordinates requested_start(answer);
        bool found = false;
        for(const Coordinates& candidate : candidates)
        {
          if(candidate == requested_start)
          {
            selected_start = candidate;
            found = true;
            break;
          }
        }
        if(!found)
        {
          command_line.printMessage("E_INVALID_MOVE");
          ambiguous_input_handled = true;
          turn_ended = false;
          return false;
        }
        was_ambiguous = true;
      }

      if(execute_from(selected_start, requested_type, requested_start_file,
                      requested_promotion_type, requested_capture))
      {
        if(was_ambiguous)
        {
          history_override_ = coordinateString(selected_start) + ":" + parameters_.at(0);
        }
        return true;
      }
      return false;
    };

    bool success = resolve_and_execute(parsed_move.getPieceType(), parsed_move.getStartFile(),
                                       parsed_move.getPromotionType(), parsed_move.getCapture());

    const std::string& raw_move = parameters_.at(0);
    if(!success && !ambiguous_input_handled && raw_move.size() >= 4 && raw_move[0] == 'b' &&
       (raw_move[1] == 'x' || raw_move[1] == 'X') && raw_move.find('=') == std::string::npos)
    {
      success = resolve_and_execute(PieceType::BISHOP, '\0', '\0', true);
    }
    if(!success)
    {
      if(!ambiguous_input_handled)
      {
        command_line.printMessage("E_INVALID_MOVE");
      }
      turn_ended = false;
      return false;
    }
    return true;
  }

  if(type_ == CommandType::USE)
  {
    std::string square_str = parameters_.at(0);
    if(!Coordinates::isValid(square_str))
    {
      command_line.printMessage("E_INV_PARAM_SQUARE");
      turn_ended = false;
      return false;
    }

    Coordinates coordinates(square_str);
    Square* square = board.getSquare(coordinates.getFile(), coordinates.getRank());
    Piece* piece = square ? square->getPiece() : nullptr;
    if(piece == nullptr || piece->getOwner() != active_player.getId())
    {
      command_line.printMessage("E_PLAYER_PIECE_NOT_FOUND");
      turn_ended = false;
      return false;
    }

    const std::string item_id = piece->getItemId();
    if(!isPotionId(item_id))
    {
      command_line.printMessage("E_NO_POTION_FOUND");
      turn_ended = false;
      return false;
    }

    int expected_count = expectedUseParameterCount(item_id);
    if(expected_count < 0 || static_cast<int>(parameters_.size()) != expected_count)
    {
      command_line.printMessage("E_INVALID_PARAM_COUNT_USE");
      turn_ended = false;
      return false;
    }

    if(item_id == "FREEZE")
    {
      if(!Coordinates::isValid(parameters_.at(1)))
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      Coordinates target_coordinates(parameters_.at(1));
      Square* target_square = board.getSquare(target_coordinates.getFile(), target_coordinates.getRank());
      Piece* target_piece = target_square ? target_square->getPiece() : nullptr;
      if(target_piece == nullptr)
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      int freeze_turns = (target_piece->getOwner() == active_player.getId()) ? 2 : 1;
      target_piece->setFrozenTurns(freeze_turns);
      piece->clearItem();
      board.clearEnPassant();
      turn_ended = true;
      return true;
    }

    if(item_id == "TP")
    {
      if(!Coordinates::isValid(parameters_.at(1)))
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      Coordinates target_coordinates(parameters_.at(1));
      int target_file = target_coordinates.getFile();
      int target_rank = target_coordinates.getRank();
      if(!isTeleportTargetLegal(board, active_player.getId(), target_rank, target_file))
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      std::unique_ptr<Piece> teleported = square->releasePiece();
      teleported->setCoordinates(Coordinates(target_file, target_rank));
      Square* target_square = board.getSquare(target_file, target_rank);
      target_square->setPiece(std::move(teleported));
      Piece* moved_piece = target_square->getPiece();
      if(moved_piece != nullptr)
      {
        moved_piece->clearItem();
        if(target_square->getType() != "NORMAL")
        {
          target_square->executeEffect(active_player, *moved_piece);
        }
      }
      board.clearEnPassant();
      turn_ended = true;
      return true;
    }

    if(item_id == "EVENODD")
    {
      if(!Coordinates::isValid(parameters_.at(1)) || !validParity(parameters_.at(2)))
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      Coordinates target_coordinates(parameters_.at(1));
      Square* target_square = board.getSquare(target_coordinates.getFile(), target_coordinates.getRank());
      Piece* target_piece = target_square ? target_square->getPiece() : nullptr;
      if(target_piece == nullptr || target_piece->getOwner() == active_player.getId())
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      target_piece->setEvenOddRestriction(lowerCopy(parameters_.at(2)));
      piece->clearItem();
      board.clearEnPassant();
      turn_ended = true;
      return true;
    }

    if(item_id == "LUKE")
    {
      if(!executeSkywalker(board, piece))
      {
        command_line.printMessage("E_INV_PARAM_USE");
        turn_ended = false;
        return false;
      }
      piece->clearItem();
      board.clearEnPassant();
      turn_ended = true;
      return true;
    }

    command_line.printMessage("E_NO_POTION_FOUND");
    turn_ended = false;
    return false;
  }

  if(type_ == CommandType::PASS)
  {
    turn_ended = true;
    return true;
  }

  return false;
}

Command::Command(CommandType type)
  : type_(type) {}

CommandType Command::getType() const { return type_; }

std::vector<std::string>& Command::getParameters() { return parameters_; }

const std::vector<std::string>& Command::getParameters() const { return parameters_; }

const std::string& Command::getHistoryOverride() const { return history_override_; }

void Command::setType(CommandType type) { type_ = type; }
