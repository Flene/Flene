//----------------------------------------------------------------------------------------------------------------------
/// The Game class coordinates the main game loop and engine state, managing configuration files, 
/// the setup/placement phase, turn transitions, command routing, history tracking, 
/// and end-game elo score calculations.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include "Utils.hpp"
#include "Player.hpp"
#include "Piece.hpp"
#include "Board.hpp"
#include "CommandLine.hpp"
#include "Command.hpp"

class Game
{
  private:
    int max_turns_ = 0;
    int max_mana_ = 0;
    Player white_;
    Player black_;
    Board board_;
    CommandLine command_line_;
    std::vector<std::vector<std::string>> white_history_;
    std::vector<std::vector<std::string>> black_history_;

    bool hasValidParameterCount(CommandType type, std::size_t count) const;
    Player& getPlayerById(PlayerId id);
    Player& getOpponent(Player& player);
    std::string playerName(PlayerId id) const;
    void printBoard(Player& active_player, int turn_count, bool can_pass = false);
    std::string historyText() const;
    void printHistory() const;
    void recordHistory(PlayerId id, int turn_count, const Command& command);
    std::string commandHistoryNotation(const Command& command) const;
    void finishGameResignation(Player& resigner, int turn_count);
    void finishGameDraw(const std::string& cause, int turn_count);
    void finishGameWin(Player* winner, int turn_count);
    void finishGame(Player* winner, Player* loser, bool draw, const std::string& proclamation, int turn_count);

  public:
    Game();

    Game(const Game& other) = delete;
    ~Game();

    bool checkMagicNumber(std::string &config_file_path);
    void loadGameFile(std::string &config_file_path);
    void parsePlayerSection(std::ifstream& file, PlayerId player, const std::string& header_line);
    void parsePieces(const std::string& line, PlayerId player_id);
    void parseSquareSection(std::ifstream& file);
    void parseItems(const std::string& line, Square& square);
    std::unique_ptr<Piece> createPiece(const std::string& piece_id, PlayerId owner);
    bool loadMessageFile(const std::string& path);
    std::unique_ptr<Square> createSquare(std::string name, SquareColor color);
    void setBishopInherent(int file, int rank);
    bool placementPhase();
    void start();
    std::vector<std::string> generateAllLegalMoves(Player& active_player, int turn_count, 
                            bool frightened_king_cannot_capture, const Coordinates* forced_start); // A3
    std::vector<std::string> generateAllLegalSpecials(Player& active_player, int turn_count, 
                            bool frightened_king_cannot_capture, const Coordinates* forced_start);
    int getPieceValue(PieceType type); // A3
    int evaluateMove(const std::string& full_move_str, PlayerId active_player_id);
};

#endif
