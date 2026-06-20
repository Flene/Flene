//----------------------------------------------------------------------------------------------------------------------
/// The Utils class contains some useful functions for use in other classes. These include type conversions, 
/// trimming input, converting the case of strings as well as splitting a string into tokens.
///
/// Author(s): Tutors, 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#include "Utils.hpp"

bool Utils::stringToInt(const std::string &string, int &out)
{
  std::istringstream stream(string);
  stream >> out;
  return stream.eof() && !stream.fail();
}

bool Utils::stringToSizeT(const std::string &string, std::size_t &out)
{
  std::istringstream stream(string);
  stream >> out;
  return stream.eof() && !stream.fail();
}

bool Utils::stringToFloat(const std::string &string, float &out)
{
  std::istringstream stream(string);
  stream >> out;
  return stream.eof() && !stream.fail();
}

bool Utils::stringToDouble(const std::string &string, double &out)
{
  std::istringstream stream(string);
  stream >> out;
  return stream.eof() && !stream.fail();
}

void Utils::trimStart(std::string &string)
{
  std::size_t start = string.find_first_not_of(' ');
  string = start == std::string::npos ? "" : string.substr(start);
}

void Utils::trimEnd(std::string &string)
{
  std::size_t end = string.find_last_not_of(' ');
  string = end == std::string::npos ? "" : string.substr(0, end + 1);
}

void Utils::trim(std::string &string)
{
  trimStart(string);
  trimEnd(string);
}

void Utils::toLowerCase(std::string &string)
{
  std::transform(
    string.begin(),
    string.end(),
    string.begin(),
    [](unsigned char character)
    {
      return std::tolower(character);
    }
  );
}

void Utils::toUpperCase(std::string &string)
{
  std::transform(
    string.begin(),
    string.end(),
    string.begin(),
    [](unsigned char character)
    {
      return std::toupper(character);
    }
  );
}

void Utils::tokenize(const std::string &string, std::vector<std::string> &tokens, char delimiter)
{
  std::size_t position = 0;
  std::size_t position_2 = 0;
  std::string token;
  while ((position_2 = string.find(delimiter, position)) != std::string::npos)
  {
    token = string.substr(position, position_2 - position);
    position = position_2 + 1;
    trim(token);
    if (!token.empty())
    {
      tokens.push_back(token);
    }
  }
  tokens.push_back(string.substr(position));
}
