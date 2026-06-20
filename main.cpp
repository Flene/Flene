#include "Game.hpp"
#include <string>
#include <iostream>
#include <format>

constexpr const char* INVALID_ARGUMENT_COUNT_MESSAGE = "Error: Wrong number of arguments!\n";
constexpr const char* INVALID_FILE_MESSAGE = "Error: Invalid file ({})!\n";

enum _ReturnValue_
{
  SUCCESS,
  MEMORY,
  INVALID_ARGUMENT_COUNT,
  INVALID_FILE
};

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cout << INVALID_ARGUMENT_COUNT_MESSAGE;
    return INVALID_ARGUMENT_COUNT;
  }
  
  std::string game_config_file_path = argv[1];
  std::string message_config_file_path = argv[2];

  Game game;

  if (!game.checkMagicNumber(game_config_file_path))
  {
    std::cout << std::format(INVALID_FILE_MESSAGE, game_config_file_path);
    return INVALID_FILE;
  }
  if (!game.checkMagicNumber(message_config_file_path))
  {
    std::cout << std::format(INVALID_FILE_MESSAGE, message_config_file_path);
    return INVALID_FILE;
  }
  game.loadGameFile(game_config_file_path);
  game.loadMessageFile(message_config_file_path);
  game.start();
  return 0;
}