//----------------------------------------------------------------------------------------------------------------------
/// The King class represents a standard king piece, implementing specific movement logic, castling, 
/// check and checkmate rules.
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#include "King.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include "Square.hpp"
#include <cstdlib>
#include <memory>

namespace
{
  bool isKingInCheck(King* checked_king, Board& board, bool verify_attacker_king_safety, int turn_count);

  Piece* findCastlingRook(Board& board, PlayerId owner, int rank, int start_file, int direction)
  {
    for(int file = start_file + direction; file >= 0 && file < 8; file += direction)
    {
      Square* square = board.getSquare(file, rank);
      if(square == nullptr) return nullptr;
      Piece* piece = square->getPiece();
      if(piece == nullptr) continue;
      if(piece->getOwner() == owner && piece->getType() == PieceType::ROOK)
      {
        return piece;
      }
      return nullptr;
    }
    return nullptr;
  }

  bool basicCastlingData(Board& board, King* king, int start_rank, int start_file, int target_rank, int target_file,
                         Piece*& rook, int& rook_file, int& skipped_file, int& direction)
  {
    rook = nullptr;
    rook_file = -1;
    skipped_file = -1;
    direction = 0;

    if(king == nullptr) return false;
    if(target_rank != start_rank) return false;
    if(std::abs(target_file - start_file) != 2) return false;
    if(king->hasMoved() || king->getFrozen() || king->isInvincible()) return false;

    direction = target_file > start_file ? 1 : -1;
    skipped_file = start_file + direction;

    Square* source_square = board.getSquare(start_file, start_rank);
    Square* target_square = board.getSquare(target_file, target_rank);
    if(source_square == nullptr || target_square == nullptr) return false;
    if(source_square->getPiece() != king) return false;

    rook = findCastlingRook(board, king->getOwner(), start_rank, start_file, direction);
    if(rook == nullptr || rook->hasMoved() || rook->getFrozen() || rook->isInvincible()) return false;
    rook_file = rook->getCoordinates().getFile();

    Piece* target_piece = target_square->getPiece();
    if(target_piece != nullptr && target_piece != rook) return false;

    for(int file = start_file + direction; file != rook_file; file += direction)
    {
      Square* square = board.getSquare(file, start_rank);
      if(square == nullptr) return false;
      if(square->getPiece() != nullptr) return false;
    }
    return true;
  }

  bool simulateCastlingAndCheck(Board& board, Player& active_player, King* king,
                                int start_rank, int start_file, int target_rank, int target_file,
                                bool final_position, int turn_count)
  {
    Piece* rook = nullptr;
    int rook_file = -1;
    int skipped_file = -1;
    int direction = 0;
    if(!basicCastlingData(board, king, start_rank, start_file, target_rank, target_file,
                          rook, rook_file, skipped_file, direction))
    {
      return true;
    }

    Square* source_square = board.getSquare(start_file, start_rank);
    Square* skipped_square = board.getSquare(skipped_file, start_rank);
    Square* target_square = board.getSquare(target_file, target_rank);
    Square* rook_square = board.getSquare(rook_file, start_rank);
    if(source_square == nullptr || skipped_square == nullptr || target_square == nullptr || rook_square == nullptr) return true;

    std::unique_ptr<Piece> rook_piece;
    Coordinates original_rook_coordinates(0, 0);
    if(final_position || rook_file == skipped_file)
    {
      rook_piece = rook_square->releasePiece();
      if(rook_piece) original_rook_coordinates = rook_piece->getCoordinates();
    }

    std::unique_ptr<Piece> king_piece = source_square->releasePiece();
    if(!king_piece)
    {
      if(rook_piece) rook_square->setPiece(std::move(rook_piece));
      return true;
    }

    Coordinates original_king_coordinates = king_piece->getCoordinates();
    Square* king_test_square = final_position ? target_square : skipped_square;
    const int king_test_file = final_position ? target_file : skipped_file;
    king_piece->setCoordinates(Coordinates(king_test_file, start_rank));
    king_test_square->setPiece(std::move(king_piece));

    if(final_position && rook_piece)
    {
      rook_piece->setCoordinates(Coordinates(skipped_file, start_rank));
      skipped_square->setPiece(std::move(rook_piece));
    }

    King* own_king = board.getKing(active_player.getId());
    bool king_in_check = own_king == nullptr || isKingInCheck(own_king, board, true, turn_count);

    king_piece = king_test_square->releasePiece();
    if(king_piece)
    {
      king_piece->setCoordinates(original_king_coordinates);
      source_square->setPiece(std::move(king_piece));
    }

    if(final_position)
    {
      rook_piece = skipped_square->releasePiece();
      if(rook_piece)
      {
        rook_piece->setCoordinates(original_rook_coordinates);
        rook_square->setPiece(std::move(rook_piece));
      }
    }

    if(!final_position && rook_file == skipped_file)
    {
      if(rook_piece)
      {
        rook_piece->setCoordinates(original_rook_coordinates);
        rook_square->setPiece(std::move(rook_piece));
      }
    }

    return king_in_check;
  }

  bool simulateMoveAndCheck(Board& board, Player& active_player, Piece* piece,
                            int start_rank, int start_file, int target_rank, int target_file,
                            bool is_capture, bool verify_attacker_king_safety, int turn_count)
  {
    Square* source_square = board.getSquare(start_file, start_rank);
    Square* target_square = board.getSquare(target_file, target_rank);
    if(source_square == nullptr || target_square == nullptr || piece == nullptr) return true;
    if(source_square->getPiece() != piece) return true;

    if(piece->getType() == PieceType::KING && !is_capture && start_rank == target_rank &&
       std::abs(target_file - start_file) == 2)
    {
      King* castling_king = dynamic_cast<King*>(piece);
      if(isKingInCheck(castling_king, board, true, turn_count)) return true;
      if(simulateCastlingAndCheck(board, active_player, castling_king,
                                  start_rank, start_file, target_rank, target_file,
                                  false, turn_count)) return true;
      return simulateCastlingAndCheck(board, active_player, castling_king,
                                      start_rank, start_file, target_rank, target_file,
                                      true, turn_count);
    }

    Piece* target_piece = target_square->getPiece();
    bool en_passant = false;
    Square* en_passant_captured_square = nullptr;
    Piece* en_passant_captured_piece = nullptr;

    if(target_piece != nullptr && !is_capture) return true;
    if(target_piece == nullptr && is_capture)
    {
      en_passant = piece->getType() == PieceType::PAWN &&
                   board.isEnPassantCapture(piece->getOwner(), start_rank, start_file, target_rank, target_file);
      if(!en_passant) return true;
      en_passant_captured_square = board.getEnPassantCapturedSquare(piece->getOwner(), start_rank, start_file,
                                                                     target_rank, target_file);
      en_passant_captured_piece = en_passant_captured_square ? en_passant_captured_square->getPiece() : nullptr;
      if(en_passant_captured_piece == nullptr) return true;
    }

    Piece* captured_piece_ptr = en_passant ? en_passant_captured_piece : target_piece;
    if(captured_piece_ptr != nullptr && captured_piece_ptr->hasItem("SHIELD"))
    {
      captured_piece_ptr->clearItem();
      King* own_king = board.getKing(active_player.getId());
      bool king_in_check = own_king == nullptr || isKingInCheck(own_king, board, verify_attacker_king_safety, turn_count);
      captured_piece_ptr->setItem("SHIELD");
      return king_in_check;
    }

    std::unique_ptr<Piece> captured_piece;
    if(en_passant)
    {
      captured_piece = en_passant_captured_square->releasePiece();
    }
    else if(is_capture)
    {
      captured_piece = target_square->releasePiece();
    }

    std::unique_ptr<Piece> moving_piece = source_square->releasePiece();
    if(!moving_piece)
    {
      if(captured_piece)
      {
        if(en_passant) en_passant_captured_square->setPiece(std::move(captured_piece));
        else target_square->setPiece(std::move(captured_piece));
      }
      return true;
    }

    Coordinates original_coordinates = moving_piece->getCoordinates();
    moving_piece->setCoordinates(Coordinates(target_file, target_rank));
    target_square->setPiece(std::move(moving_piece));

    King* own_king = board.getKing(active_player.getId());
    bool king_in_check = own_king == nullptr || isKingInCheck(own_king, board, verify_attacker_king_safety, turn_count);

    std::unique_ptr<Piece> moved_back = target_square->releasePiece();
    if(moved_back)
    {
      moved_back->setCoordinates(original_coordinates);
      source_square->setPiece(std::move(moved_back));
    }
    if(captured_piece)
    {
      if(en_passant) en_passant_captured_square->setPiece(std::move(captured_piece));
      else target_square->setPiece(std::move(captured_piece));
    }

    return king_in_check;
  }

  bool isKingInCheck(King* checked_king, Board& board, bool verify_attacker_king_safety, int turn_count)
  {
    if(checked_king == nullptr) return false;

    PlayerId checked_owner = checked_king->getOwner();
    Coordinates king_coordinates = checked_king->getCoordinates();

    for(int row = 0; row < 8; row++)
    {
      for(int col = 0; col < 8; col++)
      {
        Square* square = board.getSquare(col, row);
        if(square == nullptr) continue;

        Piece* piece = square->getPiece();
        if(piece == nullptr || piece->getOwner() == checked_owner) continue;
        if(turn_count > 0 && !piece->canMoveOnTurn(turn_count)) continue;

        Player attacking_player(piece->getOwner());
        int start_rank = piece->getCoordinates().getRank();
        int start_file = piece->getCoordinates().getFile();
        if(!piece->move(start_rank, start_file, king_coordinates.getRank(), king_coordinates.getFile(),
                        true, board, attacking_player, true))
        {
          continue;
        }

        if(verify_attacker_king_safety &&
           simulateMoveAndCheck(board, attacking_player, piece, start_rank, start_file,
                                king_coordinates.getRank(), king_coordinates.getFile(), true, false, turn_count))
        {
          continue;
        }

        return true;
      }
    }
    return false;
  }
}

King::King(PlayerId owner)
    : Piece(PieceType::KING, owner, "K", (owner == PlayerId::WHITE ? "♚K" : "♚k"), 1000)
{}

bool King::move(int start_rank, int start_file, int target_row, int target_col,
                bool is_capture, Board& board, Player& active_player, bool dry_run)
{
  int row_diff = std::abs(target_row - start_rank);
  int col_diff = std::abs(target_col - start_file);

  if(row_diff == 0 && col_diff == 2 && !is_capture)
  {
    Piece* rook = nullptr;
    int rook_file = -1;
    int skipped_file = -1;
    int direction = 0;
    if(!basicCastlingData(board, this, start_rank, start_file, target_row, target_col,
                          rook, rook_file, skipped_file, direction)) return false;
    if(dry_run) return true;

    Square* source_square = board.getSquare(start_file, start_rank);
    Square* target_square = board.getSquare(target_col, target_row);
    Square* skipped_square = board.getSquare(skipped_file, start_rank);
    Square* rook_square = board.getSquare(rook_file, start_rank);
    if(source_square == nullptr || target_square == nullptr || skipped_square == nullptr || rook_square == nullptr) return false;

    std::unique_ptr<Piece> rook_piece;
    if(rook_file != skipped_file)
    {
      rook_piece = rook_square->releasePiece();
    }
    std::unique_ptr<Piece> king_piece = source_square->releasePiece();
    if(!king_piece)
    {
      if(rook_piece) rook_square->setPiece(std::move(rook_piece));
      return false;
    }

    king_piece->setCoordinates(Coordinates(target_col, target_row));
    king_piece->setHasMoved(true);
    target_square->setPiece(std::move(king_piece));

    if(rook_file != skipped_file && rook_piece)
    {
      rook_piece->setCoordinates(Coordinates(skipped_file, start_rank));
      rook_piece->setHasMoved(true);
      skipped_square->setPiece(std::move(rook_piece));
    }
    else if(rook_file == skipped_file)
    {
      Piece* existing_rook = skipped_square->getPiece();
      if(existing_rook) existing_rook->setHasMoved(true);
    }

    Piece* moved_king = target_square->getPiece();
    if(moved_king != nullptr)
    {
      target_square->executeEffect(active_player, *moved_king);
    }
    return true;
  }

  if(row_diff > 1 || col_diff > 1 || (row_diff == 0 && col_diff == 0)) return false;
  return movePieceOnBoard(start_rank, start_file, target_row, target_col, is_capture, board, active_player, false, dry_run);
}

bool King::inCheck(Board& board, Player& active_player, int turn_count)
{
  (void)active_player;
  return isKingInCheck(this, board, true, turn_count);
}

bool King::wouldBeInCheckAfterMove(Board& board, Player& active_player, Piece* piece,
                                   int start_rank, int start_file, int target_rank, int target_file,
                                   bool is_capture, int turn_count)
{
  return simulateMoveAndCheck(board, active_player, piece, start_rank, start_file,
                              target_rank, target_file, is_capture, true, turn_count);
}

bool King::hasLegalMove(Board& board, Player& active_player, int turn_count,
                        bool frightened_king_cannot_capture)
{
  for(int start_rank = 0; start_rank < 8; ++start_rank)
  {
    for(int start_file = 0; start_file < 8; ++start_file)
    {
      Square* source_square = board.getSquare(start_file, start_rank);
      Piece* piece = source_square ? source_square->getPiece() : nullptr;
      if(piece == nullptr || piece->getOwner() != owner_) continue;
      if(!piece->canMoveOnTurn(turn_count)) continue;

      for(int target_rank = 0; target_rank < 8; ++target_rank)
      {
        for(int target_file = 0; target_file < 8; ++target_file)
        {
          if(start_rank == target_rank && start_file == target_file) continue;

          Square* target_square = board.getSquare(target_file, target_rank);
          if(target_square == nullptr) continue;

          bool is_capture = target_square->getPiece() != nullptr;
          if(piece->getType() == PieceType::PAWN && target_square->getPiece() == nullptr &&
             board.isEnPassantCapture(piece->getOwner(), start_rank, start_file, target_rank, target_file))
          {
            is_capture = true;
          }
          if(frightened_king_cannot_capture && piece == this && piece->getPieceId() == "KFRT" && is_capture)
          {
            continue;
          }
          if(!piece->move(start_rank, start_file, target_rank, target_file,
                          is_capture, board, active_player, true))
          {
            continue;
          }

          if(!wouldBeInCheckAfterMove(board, active_player, piece, start_rank, start_file,
                                      target_rank, target_file, is_capture, turn_count))
          {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool King::inCheckmate(Board& board, Player& active_player, int turn_count,
                        bool frightened_king_cannot_capture)
{
  return inCheck(board, active_player, turn_count) &&
         !hasLegalMove(board, active_player, turn_count, frightened_king_cannot_capture);
}

bool King::inStalemate(Board& board, Player& active_player, bool can_pass, int turn_count,
                         bool frightened_king_cannot_capture)
{
  if(can_pass) return false;
  return !inCheck(board, active_player, turn_count) &&
         !hasLegalMove(board, active_player, turn_count, frightened_king_cannot_capture);
}
