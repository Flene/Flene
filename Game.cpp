//----------------------------------------------------------------------------------------------------------------------
/// The Game class coordinates the main game loop and engine state, managing configuration files, 
/// the setup/placement phase, turn transitions, command routing, history tracking, 
/// and end-game elo score calculations.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Game.hpp"
#include "Search.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <set>
#include <functional>
#include <limits>
#include <vector>
#include <chrono>
#include <unordered_map>

#include "Pawn.hpp"
#include "GoldenPawn.hpp"
#include "ImpatientPawn.hpp"
#include "StubbornPawn.hpp"
#include "NervousPawn.hpp"
#include "ExplosivePawn.hpp"
#include "Rook.hpp"
#include "InvincibleRook.hpp"
#include "PainterRook.hpp"
#include "Knight.hpp"
#include "JumpyKnight.hpp"
#include "IceKnight.hpp"
#include "Bishop.hpp"
#include "ColorBlindBishop.hpp"
#include "PreacherBishop.hpp"
#include "Queen.hpp"
#include "FlipperQueen.hpp"
#include "JumpyQueen.hpp"
#include "HungryQueen.hpp"
#include "King.hpp"
#include "FrightenedKing.hpp"
#include "ArcherKing.hpp"
#include "Board.hpp"
#include "ManaSquare.hpp"
#include "BoostSquare.hpp"
#include "SpawnSquare.hpp"


Game::Game()
  : white_(PlayerId::WHITE), black_(PlayerId::BLACK) {}

bool Game::checkMagicNumber(std::string &config_file_path)
{
  std::string line;
  std::ifstream file(config_file_path);
  if(!file.is_open())
  {
    return false;
  }
  std::getline(file, line);
  Utils::trim(line);
  if(line != "MESSAGE" && line != "GAME")
  {
    return false;
  }
  return true;
}

Game::~Game() = default;

void Game::loadGameFile(std::string &config_file_path)
{
  std::ifstream file(config_file_path);
  std::string line;

  while (std::getline(file, line))
  {
    Utils::trim(line);
    if (line.empty() || line == "GAME") continue;

    // parse turns
    if (line.find("turns:") == 0)
    {
      max_turns_ = std::stoi(line.substr(6));
    }
    // parse mana
    else if (line.find("mana:") == 0)
    {
      std::string values = line.substr(5);
      size_t slash = values.find('/');
      int initial_mana = std::stoi(values.substr(0, slash));
      max_mana_ = std::stoi(values.substr(slash + 1));
      white_.setManaPoolSize(max_mana_);
      black_.setManaPoolSize(max_mana_);
      white_.setMana(initial_mana);
      black_.setMana(initial_mana);
    }
    // parse players
    else if (line.find("White") != std::string::npos)
    {
      parsePlayerSection(file, PlayerId::WHITE, line);
    }
    else if (line.find("Black") != std::string::npos)
    {
      parsePlayerSection(file, PlayerId::BLACK, line);
    }
    else if (line == "Squares")
    {
      parseSquareSection(file);
    }
  }
}

void Game::parsePlayerSection(std::ifstream& file, PlayerId player, const std::string& header_line)
{
  // parse elo
  size_t elo_pos = header_line.find("Elo = ");

  int elo = std::stoi(header_line.substr(elo_pos + 6, header_line.find(')') - (elo_pos + 6)));

  if (player == PlayerId::WHITE) white_.setEloScore(elo);
  else black_.setEloScore(elo);

  std::string line;

  // skip {
  std::getline(file, line);

  // front rank
  std::getline(file, line);
  parsePieces(line, player);

  // back rank
  std::getline(file, line);
  parsePieces(line, player);

  // skip }
  std::getline(file, line);
}


void Game::parsePieces(const std::string& line, PlayerId player_id)
{
  std::stringstream ss(line);
  std::string token;
  Player& player = (player_id == PlayerId::WHITE) ? white_ : black_;

  while (std::getline(ss, token, ','))
  {
    Utils::trim(token);
    size_t x_pos = token.find('x');

   
    if (x_pos == std::string::npos)
    {
      player.addPieceToQueue(createPiece(token, player_id));
    }
   
    else
    {
      int count = std::stoi(token.substr(0, x_pos));
      std::string piece_id = token.substr(x_pos + 1);
          
      for (int i = 0; i < count; i++)
      {
        player.addPieceToQueue(createPiece(piece_id, player_id));
      }
    }
  }
}

void Game::parseSquareSection(std::ifstream& file)
{
  std::string line;
  std::string coordinate;
  std::string square_name;
  // skip {
  std::getline(file, line);

  while (std::getline(file, line))
  {
    Utils::trim(line);
    if (line == "}") break;

    // coordinate
    if (!line.empty() && line.back() == ':' && line.find("-") == std::string::npos)
    {
      coordinate = line.substr(0, line.size() - 1);
    }
    // name
    else if (line.find("- name:") == 0)
    {
      square_name = line.substr(8);
      Utils::trim(square_name);
      Coordinates coordinates(coordinate);
      int col = coordinates.getFile(); 
      int row = coordinates.getRank();

      SquareColor color = ((col + row) % 2 == 0) ? SquareColor::BLACK : SquareColor::WHITE;
      auto new_square = createSquare(square_name, color);
      board_.placeSquare(col, row, std::move(new_square));
    }
    // item list
    else if (line.find("- list:") == 0)
    {
      Coordinates coordinates(coordinate);
      Square* square = board_.getSquare(coordinates.getFile(), coordinates.getRank());
      if (square) parseItems(line.substr(8), *square);
    }
  }
}

void Game::parseItems(const std::string& line, Square& square)
{
  std::stringstream ss(line);
  std::string token;

  while (std::getline(ss, token, ','))
  {
    Utils::trim(token);
    size_t x_pos = token.find('x');

    if (x_pos == std::string::npos)
    {
      square.addSpawnItem(token);
    }
    else
    {
      int count = std::stoi(token.substr(0, x_pos));

      std::string item_id = token.substr(x_pos + 1);
      Utils::trim(item_id);

      for (int i = 0; i < count; i++)
      {
        square.addSpawnItem(item_id);
      }
    }
  }
}

std::unique_ptr<Piece> Game::createPiece(const std::string& piece_id, PlayerId owner)
{
  switch(piece_id[0])
  {
    case 'P':
      if (piece_id == "P") return std::make_unique<Pawn>(owner);
      else if (piece_id == "PGLD") return std::make_unique<GoldenPawn>(owner);
      else if (piece_id == "PIPT") return std::make_unique<ImpatientPawn>(owner);
      else if (piece_id == "PSTB") return std::make_unique<StubbornPawn>(owner);
      else if (piece_id == "PNRV") return std::make_unique<NervousPawn>(owner);
      else if (piece_id == "PEXP") return std::make_unique<ExplosivePawn>(owner);
      break;

    case 'R':
      if (piece_id == "R") return std::make_unique<Rook>(owner);
      else if (piece_id == "RINV") return std::make_unique<InvincibleRook>(owner);
      else if (piece_id == "RPNT") return std::make_unique<PainterRook>(owner);
      break;

    case 'N':
      if (piece_id == "N") return std::make_unique<Knight>(owner);
      else if (piece_id == "NJMP") return std::make_unique<JumpyKnight>(owner);
      else if (piece_id == "NICE") return std::make_unique<IceKnight>(owner);
      break;

    case 'B':
      if (piece_id == "B") return std::make_unique<Bishop>(owner);
      else if (piece_id == "BCLR") return std::make_unique<ColorBlindBishop>(owner);
      else if (piece_id == "BPRC") return std::make_unique<PreacherBishop>(owner);
      break;

    case 'Q':
      if (piece_id == "Q") return std::make_unique<Queen>(owner);
      else if (piece_id == "QFLP") return std::make_unique<FlipperQueen>(owner);
      else if (piece_id == "QJMP") return std::make_unique<JumpyQueen>(owner);
      else if (piece_id == "QHNGR") return std::make_unique<HungryQueen>(owner);
      break;

    case 'K':
      if (piece_id == "K") return std::make_unique<King>(owner);
      else if (piece_id == "KFRT") return std::make_unique<FrightenedKing>(owner);
      else if (piece_id == "KARC") return std::make_unique<ArcherKing>(owner);
      break;
  }
  return nullptr;
}

std::unique_ptr<Square> Game::createSquare(std::string name, SquareColor color)
{
  if (name == "MANA") return std::make_unique<ManaSquare>(color);
  if (name == "BOOST") return std::make_unique<BoostSquare>(color);
  if (name == "SPAWN") return std::make_unique<SpawnSquare>(color);
  return std::make_unique<Square>(color);
}

bool Game::loadMessageFile(const std::string& path)
{
  std::ifstream file(path);
  if(!file.is_open()) return false;
  std::string line;

  // magic number
  std::getline(file, line);
  if (line != "MESSAGE") return false;

  while(std::getline(file, line))
  {
    if(line.empty()) continue;
    size_t colon = line.find(':');
    if(colon == std::string::npos) continue;
    std::string key = line.substr(0, colon);
    Utils::trim(key);
    std::string text = line.substr(colon + 1);

    std::string final_message;
    if(key[0] == 'E')
    {
      final_message = "[ERROR] " + text + '\n';
    }
    else
    {
      final_message = text + "\n";
    }
    command_line_.addMessage(key, final_message);
  }
  return true;
}

void Game::setBishopInherent(int file, int rank)
{
  Square* sq = board_.getSquare(file, rank);
  if (!sq) return;
  Piece* piece = sq->getPiece();
  Bishop* bishop = dynamic_cast<Bishop*>(piece);
  if (!bishop) return;

  std::string type = sq->getType();
  SquareColor inherent;
  if (type == "SPAWN" || type == "BOOST" || type == "MANA")
  {
    inherent = (bishop->getOwner() == PlayerId::WHITE) ? SquareColor::WHITE : SquareColor::BLACK;
  }
  else
  {
    inherent = sq->getColor();
  }
  bishop->setInherentColor(inherent);
}

bool Game::placementPhase()
{
  // place front ranks
  for (int col = 0; col < 8; col++)
  {
    // white rank
    board_.placePiece(col, 1, white_.popNextPiece());
    setBishopInherent(col, 1);
    // black rank
    board_.placePiece(col, 6, black_.popNextPiece());
    setBishopInherent(col, 6);
  }
  int placement_index = 0;

  while (white_.peekNextPiece() != nullptr || black_.peekNextPiece() != nullptr)
  {
    Player* player;
    switch (placement_index % 4)
    {
      case 0: player = &white_; break;
      case 1: player = &black_; break;
      case 2: player = &black_; break;
      case 3: player = &white_; break;
    }
    Piece* piece_to_place = player->peekNextPiece();
    if (piece_to_place == nullptr)
    {
      placement_index++;
      continue;
    }
    int remaining = player->countPieces(piece_to_place->getPieceId());
    std::string input;

    std::cout << "Where do you want to place " << piece_to_place->getPieceId()
              << " (" << remaining << " remaining)?\n";
    command_line_.printPrompt(player->getId());
    std::getline(std::cin, input);

    // A3
    if (input == "play") 
    {
      int expected_rank = (player->getId() == PlayerId::WHITE) ? 0 : 7;
      std::string chosen_square = "a1";

      for (int file = 0; file < 8; ++file) 
      {
        Square* sq = board_.getSquare(file, expected_rank);
        if (sq && !sq->getPiece()) 
        {
          char file_char = 'a' + file;
          char rank_char = '1' + expected_rank;
          chosen_square = std::string(1, file_char) + rank_char;
          break;
        }
      }
      std::cout << "AI: " << chosen_square << "\n";
      input = chosen_square;
    }

    if(input == "quit" || std::cin.eof()) return false;

    auto is_auto = [](const std::string& s)
    {
      std::string cmp;
      for (auto ch : s) cmp += std::tolower(ch);
      return cmp == "auto";
    };
    if (is_auto(input))
    {
      int rank = (player->getId() == PlayerId::WHITE) ? 0 : 7;
      // place all remaining pieces
      while (player->peekNextPiece() != nullptr) 
      {
        bool placed = false;
        for (int file = 0; file < 8; ++file)
        {
          Square* sq = board_.getSquare(file, rank);
          if (sq && !sq->getPiece())
          {
            board_.placePiece(file, rank, player->popNextPiece());
            setBishopInherent(file, rank);
            placed = true;
            break; 
          }
        }
        if (!placed) break;  // rank is full
      }
      placement_index++; 
      continue;
    }
    // otherwise, manual loop
    while (true)
    {
      if (!Coordinates::isValid(input))
      {
        std::cout << "";
        std::cout << "Where do you want to place " << piece_to_place->getPieceId()
                  << " (" << remaining << " remaining)?\n";
        command_line_.printPrompt(player->getId());
        std::getline(std::cin, input);
        if(input == "quit" || std::cin.eof()) return false;
        if (is_auto(input)) break; 
        continue;
      }
      Coordinates coordinates(input);
      int file = coordinates.getFile();
      int rank = coordinates.getRank();
      int expected_rank = (player->getId() == PlayerId::WHITE) ? 0 : 7;
      if (rank != expected_rank)
      {
        std::cout << "" << (player->getId() == PlayerId::WHITE ? "1" : "8") << ").\n";
        std::cout << "Where do you want to place " << piece_to_place->getPieceId()
                  << " (" << remaining << " remaining)?\n";
        command_line_.printPrompt(player->getId());
        std::getline(std::cin, input);
        if(input == "quit" || std::cin.eof()) return false;
        if (is_auto(input)) break;
        continue;
      }

      Square* sq = board_.getSquare(file, rank);
      if (!sq || sq->getPiece())
      {
        std::cout << "";
        std::cout << "Where do you want to place " << piece_to_place->getPieceId()
                  << " (" << remaining << " remaining)?\n";
        command_line_.printPrompt(player->getId());
        std::getline(std::cin, input);
        if(input == "quit" || std::cin.eof()) return false;
        if (is_auto(input)) break;
        continue;
      }
      board_.placePiece(file, rank, player->popNextPiece());
      setBishopInherent(file, rank);
      break;
    }
    placement_index++;
  }
  return true;
}


bool Game::hasValidParameterCount(CommandType type, std::size_t count) const
{
  switch(type)
  {
    case CommandType::QUIT:
    case CommandType::BOARD:
    case CommandType::HELP:
    case CommandType::PASS:
    case CommandType::RESIGN:
    case CommandType::DRAW:
    case CommandType::HISTORY:
      return count == 0;
    case CommandType::WHOAMI:
    case CommandType::PLAY:
      return count == 0;
    case CommandType::INFO:
    case CommandType::PRISON:
    case CommandType::MOVE:
      return count == 1;
    case CommandType::SPECIAL:
    case CommandType::USE:
      return count >= 1;
    case CommandType::UNKNOWN:
      return true;
  }
  return false;
}

Player& Game::getPlayerById(PlayerId id)
{
  return id == PlayerId::WHITE ? white_ : black_;
}

Player& Game::getOpponent(Player& player)
{
  return player.getId() == PlayerId::WHITE ? black_ : white_;
}

std::string Game::playerName(PlayerId id) const
{
  return id == PlayerId::WHITE ? "White" : "Black";
}

void Game::printBoard(Player& active_player, int turn_count, bool can_pass)
{
  command_line_.printMessage("D_CHESSBOARD_BORDER");
  command_line_.printMessage("D_BORDER_D");
  board_.print(active_player.getId(), turn_count, max_turns_, white_.getMana(), black_.getMana(), max_mana_, can_pass);
  command_line_.printMessage("D_BORDER_D");
}

std::string Game::historyText() const
{
  std::ostringstream output;
  std::size_t rounds = std::max(white_history_.size(), black_history_.size());

  for(std::size_t round = 0; round < rounds; ++round)
  {
    const std::vector<std::string> empty;
    const std::vector<std::string>& white_moves = round < white_history_.size() ? white_history_[round] : empty;
    const std::vector<std::string>& black_moves = round < black_history_.size() ? black_history_[round] : empty;
    std::size_t lines = std::max(white_moves.size(), black_moves.size());
    if(lines == 0) lines = 1;

    for(std::size_t line = 0; line < lines; ++line)
    {
      if(line == 0)
      {
        output << std::left << std::setw(3) << (round + 1);
      }
      else
      {
        output << "   ";
      }
      output << "| ";
      output << std::left << std::setw(8) << (line < white_moves.size() ? white_moves[line] : "");
      output << " | ";
      output << std::left << std::setw(8) << (line < black_moves.size() ? black_moves[line] : "");
      output << " |\n";
    }
  }
  return output.str();
}

void Game::printHistory() const
{
  command_line_.printMessage("D_BORDER_HISTORY");
  std::cout << "\n";
  command_line_.printMessage("D_HISTORY_HEADER");
  std::cout << historyText();
  std::cout << "\n";
  command_line_.printMessage("D_BORDER_D");
}

std::string Game::commandHistoryNotation(const Command& command) const
{
  const std::vector<std::string>& params = command.getParameters();
  if(command.getType() == CommandType::MOVE && !params.empty())
  {
    std::string move = command.getHistoryOverride().empty() ? params.at(0) : command.getHistoryOverride();
    for(char& c : move)
    {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    std::size_t equals_pos = move.find('=');
    if(equals_pos != std::string::npos && equals_pos + 1 < move.size())
    {
      move[equals_pos + 1] = static_cast<char>(std::toupper(static_cast<unsigned char>(move[equals_pos + 1])));
    }
    return move;
  }
  if(command.getType() == CommandType::SPECIAL && !params.empty())
  {
    std::string square = params.at(0);
    Utils::toLowerCase(square);
    return "S" + square;
  }
  if(command.getType() == CommandType::USE && !params.empty())
  {
    std::string square = params.at(0);
    Utils::toLowerCase(square);
    return "*" + square;
  }
  if(command.getType() == CommandType::PASS)
  {
    return "pass";
  }
  return "";
}

void Game::recordHistory(PlayerId id, int turn_count, const Command& command)
{
  std::string notation = commandHistoryNotation(command);
  if(notation.empty()) return;

  std::vector<std::vector<std::string>>& history = id == PlayerId::WHITE ? white_history_ : black_history_;
  if(history.size() < static_cast<std::size_t>(turn_count))
  {
    history.resize(turn_count);
  }
  history[turn_count - 1].push_back(notation);
}

void Game::finishGameResignation(Player& resigner, int turn_count)
{
  King* king = board_.getKing(resigner.getId());
  bool frightened_capture_restricted = king != nullptr && king->getPieceId() == "KFRT" &&
                                       king->inCheck(board_, resigner, turn_count);
  if(king != nullptr && king->inStalemate(board_, resigner, false, turn_count, frightened_capture_restricted))
  {
    finishGameDraw("of stalemate", turn_count);
    return;
  }

  Player& winner = getOpponent(resigner);
  if(king != nullptr && king->inCheckmate(board_, resigner, turn_count, frightened_capture_restricted))
  {
    finishGameWin(&winner, turn_count);
    return;
  }

  std::ostringstream proclamation;
  proclamation << "Oh I see one of you couldn't take the pressure...\n";
  proclamation << "Well done to " << playerName(winner.getId()) << " for making "
               << playerName(resigner.getId()) << " resign in " << turn_count << " turns.\n";
  finishGame(&winner, &resigner, false, proclamation.str(), turn_count);
}

void Game::finishGameDraw(const std::string& cause, int turn_count)
{
  std::ostringstream proclamation;
  proclamation << "This game ended in a draw because " << cause << ".\n";
  proclamation << "Thank you for playing " << turn_count << " turns.\n";
  finishGame(nullptr, nullptr, true, proclamation.str(), turn_count);
}

void Game::finishGameWin(Player* winner, int turn_count)
{
  Player& loser = getOpponent(*winner);
  std::string winner_color = (winner->getId() == PlayerId::WHITE ? "White" : "Black");
  std::ostringstream proclamation;
  proclamation << "This was a great match! Well done to you both!\n";
  proclamation << "Good job to "<< winner_color <<" for winning in "<< turn_count <<" turns.\n";
  finishGame(winner, &loser, false, proclamation.str(), turn_count);
}

void Game::finishGame(Player* winner, Player* loser, bool draw, const std::string& proclamation, int turn_count)
{
  (void)turn_count;
  double q_white = std::pow(10.0, white_.getEloScore() / 400.0);
  double q_black = std::pow(10.0, black_.getEloScore() / 400.0);
  double expected_white = q_white / (q_white + q_black);
  double expected_black = 1.0 - expected_white;

  double score_white = 0.5;
  double score_black = 0.5;
  if(!draw && winner != nullptr && loser != nullptr)
  {
    score_white = winner->getId() == PlayerId::WHITE ? 1.0 : 0.0;
    score_black = winner->getId() == PlayerId::BLACK ? 1.0 : 0.0;
  }

  int new_white = static_cast<int>(std::floor(white_.getEloScore() + 32.0 * (score_white - expected_white)));
  int new_black = static_cast<int>(std::floor(black_.getEloScore() + 32.0 * (score_black - expected_black)));

  std::ostringstream elo_section;
  elo_section << "\n";
  elo_section << "The new Elo scores are:\n";
  elo_section << " - White: " << new_white << "\n";
  elo_section << " - Black: " << new_black << "\n";
  elo_section << "\n";

  std::cout << proclamation;
  std::cout << elo_section.str();

  while(true)
  {
    std::cout << "Enter the output file name\n";
    std::cout << " > ";
    std::cout.flush();

    std::string path;
    if(!std::getline(std::cin, path)) return;
    Utils::trim(path);
    if(path.empty()) return;

    std::ofstream output_file(path);
    if(!output_file.is_open())
    {
      command_line_.printMessage("E_INVALID_PATH");
      continue;
    }
    output_file << proclamation;
    output_file << elo_section.str();
    output_file << command_line_.getMessages().at("D_BORDER_HISTORY") << "\n";
    output_file << command_line_.getMessages().at("D_HISTORY_HEADER");
    output_file << historyText();
    output_file << "\n" << command_line_.getMessages().at("D_BORDER_HISTORY");
    return;
  }
}

void Game::start()
{
  Player* active_player = &white_;
  int turn_count = 1;
  bool game_finished = false;

  command_line_.printMessage("D_BORDER_D");
  command_line_.printMessage("D_WELCOME");
  command_line_.printMessage("D_BORDER_D");

  if(!placementPhase()) return;

  while(turn_count <= max_turns_ && !game_finished)
  {
    if(active_player->getId() == PlayerId::WHITE)
    {
      board_.spawnItemsForRound(turn_count);
    }

    active_player->addMana(1 + board_.countPiecesOnManaSquares(active_player->getId()));

    std::set<std::string> failed_ai_commands;

    bool turn_ended = false;
    bool extra_move = false;
    bool print_board_next_prompt = true;
    bool has_restricted_extra_piece = false;
    bool frightened_extra_move_used = false;
    Coordinates restricted_extra_piece(0, 0);
    King* turn_start_king = board_.getKing(active_player->getId());
    bool frightened_king_cannot_capture = turn_start_king != nullptr &&
                                          turn_start_king->getPieceId() == "KFRT" &&
                                          turn_start_king->inCheck(board_, *active_player, turn_count);

    while(!turn_ended && !game_finished)
    {
      if(board_.getActive_() && print_board_next_prompt)
      {
        printBoard(*active_player, turn_count, extra_move);
        print_board_next_prompt = false;
      }

      std::cout << '\n';
      command_line_.printPrompt(active_player->getId());

      std::string input;
      std::getline(std::cin, input);
      
      // A3: whoami shortcut
      if (input == "whoami") 
      {
        std::cout << "Team: idk\n";
        continue;
      }

      // AI play branch: delegate to SearchEngine
      if (input == "play")
      {
        active_player->setIsAI(true);
        const int MAX_DEPTH = 6;
        const int TIME_LIMIT_MS = 2000; // 2s
        SearchEngine search(*this, board_, MAX_DEPTH, TIME_LIMIT_MS);
        std::string best = search.findBestMove(*active_player, getOpponent(*active_player),
                                               turn_count, frightened_king_cannot_capture, failed_ai_commands);
        std::cout << "AI: " << best << "\n";
        input = best;
      }
      else active_player->setIsAI(false);
      
      if(std::cin.eof()) return;
      Utils::trim(input);

      std::vector<std::string> tokens;
      Utils::tokenize(input, tokens, ' ');
      if(tokens.empty()) continue;

      Command cmd(tokens);
      CommandType type = cmd.getType();
      const std::vector<std::string>& params = cmd.getParameters();

      if(type == CommandType::UNKNOWN)
      {
        command_line_.printMessage("E_UNKNOWN_COMMAND");
        continue;
      }

      if(!hasValidParameterCount(type, params.size()))
      {
        command_line_.printMessage("E_INVALID_PARAM_COUNT");
        continue;
      }

      if(type == CommandType::QUIT)
      {
        return;
      }

      if(type == CommandType::BOARD)
      {
        if(board_.getActive_())
        {
          board_.setActive_(false);
        }
        else
        {
          board_.setActive_(true);
          printBoard(*active_player, turn_count, extra_move);
        }
        print_board_next_prompt = false;
        continue;
      }

      if(type == CommandType::HISTORY)
      {
        printHistory();
        continue;
      }

      if(type == CommandType::PRISON)
      {
        std::string requested_player = params.at(0);
        Utils::toLowerCase(requested_player);
        if(requested_player != "white" && requested_player != "black")
        {
          command_line_.printMessage("E_INV_PARAM_PLAYER");
          continue;
        }
        Player& prison_player = requested_player == "white" ? white_ : black_;
        cmd.executeCommand(command_line_, turn_ended, board_, prison_player, nullptr, turn_count);
        continue;
      }

      if(type == CommandType::DRAW)
      {
        Player& opponent = getOpponent(*active_player);
        std::cout << "Player " << playerName(active_player->getId())
                  << " has offered a draw. Would you like to accept? (yes/no)\n";

        command_line_.printPrompt(opponent.getId());
        std::string answer;
        std::getline(std::cin, answer);

        // A3: AI responder
        if (answer == "play") 
        {
            answer = "no"; // standard answer
            std::cout << "AI: " << answer << "\n";
        }

        if(std::cin.eof()) return;
        Utils::trim(answer);
        Utils::toLowerCase(answer);

        if(answer == "yes")
        {
          finishGameDraw("you both agreed to a draw", turn_count);
          game_finished = true;
          turn_ended = true;
          continue;
        }
        if(answer != "no")
        {
          command_line_.printMessage("E_INV_PARAM_YES_NO");
        }
        continue;
      }

      if(type == CommandType::RESIGN)
      {
        finishGameResignation(*active_player, turn_count);
        game_finished = true;
        turn_ended = true;
        continue;
      }

      if(extra_move && (type == CommandType::SPECIAL || type == CommandType::USE))
      {
        command_line_.printMessage("E_SPECIAL_USE_UNAVAILABLE");
        continue;
      }

      if(type == CommandType::PASS && !extra_move)
      {
        command_line_.printMessage("E_INVALID_PASS");
        continue;
      }

      const Coordinates* forced_start = has_restricted_extra_piece ? &restricted_extra_piece : nullptr;
      bool frightened_extra_move = false;
      if(type == CommandType::MOVE && frightened_king_cannot_capture && !frightened_extra_move_used &&
         !extra_move && turn_start_king != nullptr && !params.empty())
      {
        Move pending_move(params.at(0));
        frightened_extra_move = pending_move.isValid() && pending_move.getPieceType() == PieceType::KING;
      }
      bool command_successful = cmd.executeCommand(command_line_, turn_ended, board_, *active_player,
                                                   forced_start, turn_count, frightened_king_cannot_capture);
      if (!command_successful) 
      {
        // Record failed AI command to avoid repeating it
        failed_ai_commands.insert(input);
      }
      if(command_successful && frightened_extra_move)
      {
        turn_ended = false;
        frightened_extra_move_used = true;
      }
      if(command_successful && (type == CommandType::MOVE || type == CommandType::SPECIAL || type == CommandType::USE || type == CommandType::PASS))
      {
        recordHistory(active_player->getId(), turn_count, cmd);
      }
      if(command_successful && type == CommandType::MOVE)
      {
        const int golden_back_rank = active_player->getId() == PlayerId::WHITE ? 7 : 0;
        for(int file = 0; file < 8; ++file)
        {
          Square* square = board_.getSquare(file, golden_back_rank);
          Piece* piece = square ? square->getPiece() : nullptr;
          if(piece != nullptr && piece->getOwner() == active_player->getId() && piece->getPieceId() == "PGLD")
          {
            finishGameWin(active_player, turn_count);
            game_finished = true;
            turn_ended = true;
            break;
          }
        }
        if(game_finished) continue;
      }

      if(command_successful && (type == CommandType::MOVE || type == CommandType::SPECIAL))
      {
        PlayerId player_id = active_player->getId();
        PlayerId opponent_id = player_id == PlayerId::WHITE ? PlayerId::BLACK : PlayerId::WHITE;
        bool king_captured = active_player->kingCaptured(opponent_id);
        bool own_king_captured = active_player->kingCaptured(player_id);
        if(king_captured && own_king_captured)
        {
          finishGameDraw("you both lost your king", turn_count);
          game_finished = true;
          turn_ended = true;
          continue;
        }
        else if(king_captured)
        {
          finishGameWin(active_player, turn_count);
          game_finished = true;
          turn_ended = true;
          continue;
        }
        else if(own_king_captured)
        {
          finishGameWin(&getOpponent(*active_player), turn_count);
          game_finished = true;
          turn_ended = true;
          continue;
        }
      }
      if(command_successful && type == CommandType::MOVE && !turn_ended)
      {
        if(!cmd.getParameters().empty())
        {
          Move moved(cmd.getParameters().at(0));
          if(moved.isValid())
          {
            Coordinates target(moved.getTargetSquare());
            restricted_extra_piece = target;
            has_restricted_extra_piece = true;
          }
        }
        extra_move = true;
        print_board_next_prompt = true;
      }
      else if(command_successful && (type == CommandType::MOVE || type == CommandType::PASS))
      {
        extra_move = false;
        has_restricted_extra_piece = false;
      }
    }

    if(game_finished) break;

    if(active_player->getId() == PlayerId::WHITE) 
    {
      board_.decreaseStatusTimers(active_player->getId());
      active_player = &black_; 
    } 
    else
    {
      board_.decreaseStatusTimers(active_player->getId());
      active_player = &white_; 
      turn_count++; 
    }
  }

  if(!game_finished && turn_count > max_turns_)
  {
    finishGameDraw("too many turns were played", max_turns_);
  }
}

// A3 helper functions (used by AI and SearchEngine)
// generateAllLegalMoves, generateAllLegalSpecials, getPieceValue, evaluateMove
// These methods are unchanged from previous stages and remain below.

std::vector<std::string> Game::generateAllLegalMoves(Player& active_player, int turn_count, 
                          bool frightened_king_cannot_capture, const Coordinates* forced_start)
{
  std::vector<std::string> legal_moves;
  King* active_king = board_.getKing(active_player.getId());

  // iterate over all squares
  for (int start_r = 0; start_r < 8; ++start_r)
  {
    for (int start_c = 0; start_c < 8; ++start_c)
    {
      Square* src_sq = board_.getSquare(start_c, start_r);
      Piece* p = src_sq ? src_sq->getPiece() : nullptr;

      // is owner correct and can he play
      if (!p || p->getOwner() != active_player.getId() || !p->canMoveOnTurn(turn_count)) continue;
      if (forced_start && (forced_start->getFile() != start_c || forced_start->getRank() != start_r)) continue;
      if (p->getFrozen() || p->isInvincible()) continue;

      // iterate over all possible target squares
      for (int target_r = 0; target_r < 8; ++target_r)
      {
        for (int target_c = 0; target_c < 8; ++target_c)
        {
          if (start_r == target_r && start_c == target_c) continue;

          Square* dest_sq = board_.getSquare(target_c, target_r);
          bool is_capture = (dest_sq && dest_sq->getPiece() != nullptr);

          if (frightened_king_cannot_capture && p == active_king && p->getPieceId() == "KFRT" && is_capture) continue;

          // using dry-run logic to validate move
          if (!p->move(start_r, start_c, target_r, target_c, is_capture, board_, active_player, true)) continue;

          // using wouldBeInCheckAfterMove
          if (active_king && active_king->wouldBeInCheckAfterMove(board_, active_player, p, start_r, start_c, 
              target_r, target_c, is_capture, turn_count)) continue;

          // move is valid
          std::string move_str = "move ";
          char start_file_char  = static_cast<char>('a' + start_c);
          char target_file_char = static_cast<char>('a' + target_c);
          char target_rank_char = static_cast<char>('1' + target_r);

          if (p->getType() == PieceType::PAWN)
          {
            if (is_capture)
            {
              move_str += start_file_char;
              move_str += "x";
            }
            move_str += target_file_char;
            move_str += target_rank_char;

            // promotion
            if (target_r == 0 || target_r == 7)
            {
              move_str += "=Q";
            }
          }
          else
          {
            if (p->getType() == PieceType::ROOK)        move_str += "R";
            else if (p->getType() == PieceType::KNIGHT)  move_str += "N";
            else if (p->getType() == PieceType::BISHOP)  move_str += "B";
            else if (p->getType() == PieceType::QUEEN)   move_str += "Q";
            else if (p->getType() == PieceType::KING)    move_str += "K";

            if (is_capture) move_str += "x";

            move_str += target_file_char;
            move_str += target_rank_char;
          }
          legal_moves.push_back(move_str);
        }
      }
    }
  }
  // if nothing found
  if (legal_moves.empty())
  {
    legal_moves.push_back("pass");
  }
  return legal_moves;
}

std::vector<std::string> Game::generateAllLegalSpecials(Player& active_player, int turn_count, 
                          bool frightened_king_cannot_capture, const Coordinates* forced_start)
{
  std::set<std::string> unique_specials;
  King* active_king = board_.getKing(active_player.getId());
  int mana_count = active_player.getMana();

  // Loop skeleton similar to legal moves loop
  for (int start_r = 0; start_r < 8; ++start_r)
  {
    for (int start_c = 0; start_c < 8; ++start_c)
    {
      Square* src_sq = board_.getSquare(start_c, start_r);
      Piece* p = src_sq ? src_sq->getPiece() : nullptr;

      // checks like above + check for existence of special command
      if (!p || p->getOwner() != active_player.getId() || !p->canMoveOnTurn(turn_count)) continue;
      if (forced_start && (forced_start->getFile() != start_c || forced_start->getRank() != start_r)) continue;
      if (!p->hasSpecial()) continue;
      if (p->getFrozen() || p->isInvincible()) continue;
      auto pid = p->getPieceId();

      std::vector<std::string> available_specials = p->canSpecial(&board_);
      if (available_specials.empty()) continue;

      for (const auto& it : available_specials)
      {
        if (it.empty()) continue;
        // check mana cost heuristics: rely on CommandUtils functions
        int mana_cost = CommandUtils::specialManaCost(pid);
        if (mana_cost != 0 && mana_count < mana_cost) continue;
        std::vector<std::string> command_string;
        Utils::tokenize(it, command_string, ' ');

        std::vector<std::string> parameters;
        if (command_string.size() > 2)
        {
          parameters.assign(command_string.begin() + 1, command_string.end());
        }
        mana_cost = CommandUtils::variableManaCost(pid, Coordinates(start_c, start_r), parameters, board_, active_player);
        if (mana_count < mana_cost) continue;

        unique_specials.insert(it);
      }
    }
  }
  // if nothing found
  std::vector<std::string> legal_moves(unique_specials.begin(), unique_specials.end());
  if (legal_moves.empty())
  {
    legal_moves.push_back("pass");
  }

  return legal_moves;
}

int Game::getPieceValue(PieceType type)
{
  switch (type)
  {
    case PieceType::PAWN:   return 10;
    case PieceType::KNIGHT: return 30;
    case PieceType::BISHOP: return 30;
    case PieceType::ROOK:   return 50;
    case PieceType::QUEEN:  return 90;
    case PieceType::KING:   return 1000;
    default: return 0;
  }
}

int Game::evaluateMove(const std::string& full_move_str, PlayerId active_player_id)
{
  if (full_move_str == "pass") return -100;

  int score = 0;
  int file = -1;
  int rank = -1;
  std::size_t eq_pos = full_move_str.find('=');

  // tokenize string
  std::vector<std::string> tokens;
  Utils::tokenize(full_move_str, tokens, ' ');

  if (tokens.empty()) return 0;

  // special logic
  if (tokens[0] == "special" && tokens.size() >= 2)
  {
    score += 40; // basis value of specials

    std::string target_token = tokens.back(); 
    
    // exception for impatient pawn or invincible Rook
    if (target_token.length() == 1 || (target_token[0] >= '0' && target_token[0] <= '9'))
    {
      if (tokens.size() >= 2) target_token = tokens[1];
    }

    if (target_token.length() == 2 && target_token[0] >= 'a' && target_token[0] <= 'h' && target_token[1] >= '1' && target_token[1] <= '8')
    {
      file = target_token[0] - 'a';
      rank = target_token[1] - '1';
    }
  }
  // normal moves
  else if (tokens[0] == "move" && tokens.size() >= 2)
  {
    std::string destination = tokens.back();
    // cut '='
    if (eq_pos != std::string::npos) 
    {
      std::string move_part = full_move_str.substr(5);
      destination = move_part.substr(move_part.find(' ') + 1, 2);
    }

    if (destination.length() >= 2 && destination[0] >= 'a' && destination[0] <= 'h' && destination[1] >= '1' && destination[1] <= '8')
    {
      file = destination[0] - 'a';
      rank = destination[1] - '1';
    }
  }
  if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7)
  {
    Square* sq = board_.getSquare(file, rank);
    Piece* target_piece = sq ? sq->getPiece() : nullptr;
    
    if (target_piece && target_piece->getOwner() != active_player_id)
    {
      int piece_val = getPieceValue(target_piece->getType());
      
      // 
      if (full_move_str.find("BPRC") != std::string::npos || 
          (target_piece && full_move_str.rfind("special", 0) == 0 && piece_val > 1)) 
      {
        score += piece_val * 25; // bonus for stealing
      }
      else 
      {
        score += piece_val * 10; // normal capture
      }
    }
    
    // golden pawn wins
    bool is_back_rank = (active_player_id == PlayerId::WHITE && rank == 7) || (active_player_id == PlayerId::BLACK && rank == 0);
    if (is_back_rank && full_move_str.find("PGLD") != std::string::npos)
    {
      score += 10000;
    }

    if (eq_pos != std::string::npos) score += 80; // normal promotion bonus

    // pieces need to get forward
    if (active_player_id == PlayerId::WHITE) score += rank;
    else score += (7 - rank);
  }
  
  return score;
}