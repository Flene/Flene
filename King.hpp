//----------------------------------------------------------------------------------------------------------------------
/// The King class represents a standard king piece, implementing specific movement logic, castling, 
/// check and checkmate rules.
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef KING_HPP
#define KING_HPP

#include "Piece.hpp"
#include "Player.hpp"

class King : public Piece
{
  public:
    King(PlayerId owner);

    bool move(int start_rank, int start_file, int target_row, int target_col,
          bool is_capture, Board& board, Player& active_player, bool dry_run = false) override;

    bool inCheck(Board&, Player& active_player, int turn_count = -1);
    bool wouldBeInCheckAfterMove(Board&, Player& active_player, Piece* piece,
                                int start_rank, int start_file, int target_rank, int target_file,
                                bool is_capture, int turn_count = -1);
    bool hasLegalMove(Board&, Player& active_player, int turn_count = 1,
                      bool frightened_king_cannot_capture = false);
    bool inCheckmate(Board&, Player& active_player, int turn_count = 1,
                     bool frightened_king_cannot_capture = false);
    bool inStalemate(Board&, Player& active_player, bool can_pass = false, int turn_count = 1,
                     bool frightened_king_cannot_capture = false);
};

#endif
