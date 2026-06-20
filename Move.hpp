//----------------------------------------------------------------------------------------------------------------------
/// The Move class parses and validates chess moves written in standard notation, extracting data 
/// such as piece type, target coordinates, captures, and pawn promotions.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef MOVE_HPP
#define MOVE_HPP

#include <string>
#include "Piece.hpp"
#include "Square.hpp"

class Move
{
  private:
    PieceType piece_type_ = PieceType::PAWN;
    std::string target_square_;
    bool is_capture_ = false;
    char start_file_ = '\0';
    char promotion_type_ = '\0';
    bool valid_ = false;

  public:
    Move(std::string move_str);

    void parseMoveStr(std::string move_str);

    PieceType getPieceType() const { return piece_type_; }
    std::string getTargetSquare() const { return target_square_; }
    bool getCapture() const { return is_capture_; }
    char getStartFile() const { return start_file_; }
    char getPromotionType() const { return promotion_type_; }
    bool isValid() const { return valid_; }
};

#endif
