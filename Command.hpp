//----------------------------------------------------------------------------------------------------------------------
/// The Command class parses user input into game commands and processes their execution, handling 
/// standard moves, special abilities, potion usage, and match logic.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Utils.hpp"
#include "Player.hpp"
#include "CommandLine.hpp"
#include "Move.hpp"
#include <vector>
#include <string>

enum class CommandType { QUIT, BOARD, HELP, INFO, PRISON, SPECIAL, 
                        MOVE, USE, PASS, RESIGN, DRAW, HISTORY, UNKNOWN, WHOAMI, PLAY};

namespace CommandUtils // Declare everything all the CommandUtils functions from Commmand.cpp here
{
  std::string upperCopy(std::string value);
  std::string lowerCopy(std::string value);
  bool isPieceId(const std::string& piece_id);
  bool isSpecialPiece(const std::string& piece_id);
  int expectedSpecialParameterCount(const std::string& piece_id);
  bool isValidPieceTypeParameter(const std::string& value);
  bool isValidTurnCount(const std::string& value);
  int specialManaCost(const std::string& piece_id);
  int variableManaCost(const std::string& piece_id, const Coordinates& coordinates, const std::vector<std::string>& parameters, Board& board, Player& active_player);
  bool isPotionId(const std::string& item_id);
  bool validParity(const std::string& value);
  bool isTeleportTargetLegal(Board& board, PlayerId owner, int target_rank, int target_file);
  bool executeSkywalker(Board& board, Piece* piece);
  void printPrisonForPlayer(CommandLine& command_line, Player& player);
}

class Command
{
  private:
    CommandType type_;
    std::vector<std::string> parameters_;
    std::string history_override_;

  public:
    Command(std::vector<std::string> &input_tokens);

    Command(CommandType type);
    
    Command(const Command& other) = default;
    ~Command() = default;

    bool executeCommand(CommandLine& command_line, bool& turn_ended, Board& board, Player& active_player,
                        const Coordinates* forced_start = nullptr, int turn_count = 1,
                        bool frightened_king_cannot_capture = false);

    CommandType getType() const;
    std::vector<std::string> &getParameters();
    const std::vector<std::string> &getParameters() const;
    const std::string& getHistoryOverride() const;
    void setType(CommandType type);
};

#endif
