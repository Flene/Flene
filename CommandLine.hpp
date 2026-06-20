//----------------------------------------------------------------------------------------------------------------------
/// The CommandLine class handles all terminal-based user interactions for the game, including 
/// displaying player prompts and managing game message outputs.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#ifndef COMMANDLINE_HPP
#define COMMANDLINE_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Player.hpp"

enum class ErrorType
{
  UNKNOWN_COMMAND,
  INVALID_PARAM_COUNT,
  SPECIAL_USE_UNAVAILABLE,
  INV_PARAM_PLAYER,
  INV_PARAM_SQUARE,
  INV_PARAM_PIECE,
  PLAYER_PIECE_NOT_FOUND,
  NO_SPECIAL_POWER,
  INVALID_PARAM_COUNT_SPECIAL,
  PIECE_FROZEN,
  INV_PARAM_SPECIAL_SQUARE,
  INV_PARAM_PIECE_TYPE,
  INV_PARAM_TURN_COUNT,
  UNWAVERING_FAITH,
  INVALID_ARCHER_TARGET,
  OPPONENT_PIECE_NOT_FOUND,
  INV_PARAM_MOVE,
  INVALID_MOVE,
  INSUFFICIENT_MANA,
  NO_POTION_FOUND,
  INVALID_PARAM_COUNT_USE,
  INV_PARAM_USE,
  INVALID_PASS,
  INV_PARAM_YES_NO,
  INVALID_PATH,
};

class CommandLine
{
  private:
    static const std::string INPUT_MESSAGE;
    static const std::string INPUT_PROMPT;

    std::map<std::string, std::string> MESSAGES;

  public:
    //------------------------------------------------------------------------------------------------------------------
    /// @brief Constructor is set to default.
    CommandLine() = default;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief Copy constructor is deleted explicitly.
    CommandLine(const CommandLine &) = delete;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief Destructor is set to default.
    virtual ~CommandLine() = default;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function prints the command prompt for the current player.
    /// @param player the current player
    void printPrompt(const PlayerId& id) const;

    void addMessage(const std::string& key, const std::string& text);
    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function prints the error message for the given error type.
    /// @param error_type the type of error that occurred
    void printMessage(const std::string& key) const;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function returns the map of messages.
    std::map<std::string, std::string> getMessages() const;
};

#endif // COMMANDLINE_HPP
