//----------------------------------------------------------------------------------------------------------------------
/// The Coordinates class represents and validates standard board positions, handling 
/// conversions between string notations (e.g., "a1") and internal grid indices.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Coordinates.hpp"
#include <cctype>

Coordinates::Coordinates(int file, int rank)
  : file_(file), rank_(rank) {}

Coordinates::Coordinates(const std::string& coordinate_str)
{
  if (coordinate_str.length() < 2)
  {
    file_ = -1;
    rank_ = -1;
    return;
  }
  char file_char = std::tolower(coordinate_str[0]);
  char rank_char = coordinate_str[1];

  if (file_char >= 'a' && file_char <= 'h') file_ = file_char - 'a';
  else if (file_char >= 'A' && file_char <= 'H') file_ = file_char - 'A';
  else file_ = -1; 

  if (rank_char >= '1' && rank_char <= '8') rank_ = (rank_char - '1'); 
  else rank_ = -1; 
}

int Coordinates::getFile() const { return file_; }
int Coordinates::getRank() const { return rank_; }

void Coordinates::setFile(int file) { file_ = file; }
void Coordinates::setRank(int rank) { rank_ = rank; }

bool Coordinates::isValid(const std::string& coordinate_str)
{
  if (coordinate_str.length() != 2) return false;
  char file_char = std::tolower(coordinate_str[0]);
  char rank_char = coordinate_str[1];
  return (file_char >= 'a' && file_char <= 'h') && (rank_char >= '1' && rank_char <= '8');
}

std::string Coordinates::toString() const // This performs the constructor conversion backwards
{
  if (file_ < 0 || file_ > 7 || rank_ < 0 || rank_ > 7) return "";

  char file_char = 'a' + file_;
  char rank_char = '1' + rank_;

  return std::string{file_char, rank_char};
}

