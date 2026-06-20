//----------------------------------------------------------------------------------------------------------------------
/// The Move class parses and validates chess moves written in standard notation, extracting data 
/// such as piece type, target coordinates, captures, and pawn promotions.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "Move.hpp"
#include "Coordinates.hpp"
#include <algorithm>
#include <cctype>

namespace
{
  bool isPieceType(char c)
  {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return c == 'B' || c == 'N' || c == 'R' || c == 'Q' || c == 'K';
  }

  PieceType toPieceType(char c)
  {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if(c == 'B') return PieceType::BISHOP;
    if(c == 'N') return PieceType::KNIGHT;
    if(c == 'R') return PieceType::ROOK;
    if(c == 'Q') return PieceType::QUEEN;
    return PieceType::KING;
  }

  bool isValidPromotion(char c)
  {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return c == 'B' || c == 'N' || c == 'R' || c == 'Q';
  }

  std::string lowerCopy(std::string s)
  {
    for(char& c : s)
    {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
  }
}

Move::Move(std::string move_str)
{
  parseMoveStr(move_str);
}

void Move::parseMoveStr(std::string str)
{
  valid_ = false;
  piece_type_ = PieceType::PAWN;
  target_square_.clear();
  is_capture_ = false;
  start_file_ = '\0';
  promotion_type_ = '\0';

  if(str.empty()) return;

  size_t eq_pos = str.find('=');
  if(eq_pos != std::string::npos)
  {
    if(str.find('=', eq_pos + 1) != std::string::npos) return;
    if(eq_pos + 2 != str.size()) return;
    promotion_type_ = static_cast<char>(std::toupper(static_cast<unsigned char>(str[eq_pos + 1])));
    if(!isValidPromotion(promotion_type_)) return;
    str = str.substr(0, eq_pos);
  }

  size_t x_pos = str.find('x');
  if(x_pos == std::string::npos) x_pos = str.find('X');
  if(x_pos != std::string::npos)
  {
    if(str.find('x', x_pos + 1) != std::string::npos || str.find('X', x_pos + 1) != std::string::npos) return;
    is_capture_ = true;
  }

  if(str.size() < 2) return;
  target_square_ = lowerCopy(str.substr(str.size() - 2));
  if(!Coordinates::isValid(target_square_)) return;

  if(is_capture_)
  {
    if(x_pos != str.size() - 3) return;
    if(x_pos != 1) return;
    char first = str[0];
    if(isPieceType(first) && first != 'b')
    {
      piece_type_ = toPieceType(first);
      if(promotion_type_ != '\0') return; // only pawns promote
    }
    else
    {
      char file = static_cast<char>(std::tolower(static_cast<unsigned char>(first)));
      if(file < 'a' || file > 'h') return;
      piece_type_ = PieceType::PAWN;
      start_file_ = file;
    }
  }
  else
  {
    if(str.size() == 2)
    {
      piece_type_ = PieceType::PAWN;
    }
    else if(str.size() == 3 && isPieceType(str[0]))
    {
      piece_type_ = toPieceType(str[0]);
      if(promotion_type_ != '\0') return; // only pawns promote
    }
    else
    {
      return;
    }
  }

  valid_ = true;
}
