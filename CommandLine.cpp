//----------------------------------------------------------------------------------------------------------------------
/// The CommandLine class handles all terminal-based user interactions for the game, including 
/// displaying player prompts and managing game message outputs.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#include "CommandLine.hpp"

void CommandLine::printPrompt(const PlayerId& id) const
{
  std::string player;
  if(id == PlayerId::WHITE) player = "White";
  else player = "Black";
  std::cout << player << " > ";
  std::cout.flush();
}

void CommandLine::addMessage(const std::string& key, const std::string& text)
{
  MESSAGES[key] = text;
}

void CommandLine::printMessage(const std::string& key) const
{
  if (MESSAGES.count(key))
  {
    std::cout << MESSAGES.at(key);
  }
}

std::map<std::string, std::string> CommandLine::getMessages() const
{
  return MESSAGES;
}
