//----------------------------------------------------------------------------------------------------------------------
/// The Board class represents the chess board, managing grid coordinates, square types, 
/// piece placement, item spawning, en passant rules and board printing.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#include "Board.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include "Piece.hpp"
#include "Square.hpp"
#include "Utils.hpp"
#include "Command.hpp"
#include "Move.hpp"
#include "CommandLine.hpp"
#include "Pawn.hpp"
#include <vector>
#include <limits>

Board::Board()
{
    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            SquareColor color = ((r + c) % 2 == 0) ? SquareColor::BLACK : SquareColor::WHITE;
            grid_[r][c] = std::make_unique<Square>(color);
        }
    }
}

Board::~Board() = default;

void Board::placeSquare(int file, int rank, std::unique_ptr<Square> new_square)
{
    if (rank >= 0 && rank < 8 && file >= 0 && file < 8)
    {
        grid_[rank][file] = std::move(new_square);
    }
}

void Board::placePiece(int file, int rank, std::unique_ptr<Piece> piece)
{
    if (rank >= 0 && rank < 8 && file >= 0 && file < 8)
    {
        piece->setCoordinates(Coordinates(file, rank));
        piece->initializeOnSquare(grid_[rank][file].get());
        grid_[rank][file]->setPiece(std::move(piece));
    }
}

Square* Board::getSquare(int file, int rank)
{
    if (rank < 0 || rank >= 8 || file < 0 || file >= 8)
        return nullptr;
    return grid_[rank][file].get();
}

const Square* Board::getSquare(int file, int rank) const
{
    if (rank < 0 || rank >= 8 || file < 0 || file >= 8)
        return nullptr;
    return grid_[rank][file].get();
}

void Board::decreaseStatusTimers(PlayerId owner)
{
    for(int r = 0; r < 8; ++r)
    {
        for(int c = 0; c < 8; ++c)
        {
            Piece* piece = grid_[r][c] ? grid_[r][c]->getPiece() : nullptr;
            if(piece != nullptr && piece->getOwner() == owner)
            {
                piece->decreaseStatusTimers();
            }
        }
    }
}


int Board::countPiecesOnManaSquares(PlayerId owner) const
{
    int count = 0;
    for(int r = 0; r < 8; ++r)
    {
        for(int c = 0; c < 8; ++c)
        {
            const Square* square = grid_[r][c].get();
            const Piece* piece = square ? square->getPiece() : nullptr;
            if(square != nullptr && square->getType() == "MANA" &&
               piece != nullptr && piece->getOwner() == owner)
            {
                ++count;
            }
        }
    }
    return count;
}

void Board::spawnItemsForRound(int round_number)
{
    if(round_number <= 0 || round_number % 3 != 0) return;
    for(int r = 0; r < 8; ++r)
    {
        for(int c = 0; c < 8; ++c)
        {
            Square* square = grid_[r][c].get();
            if(square != nullptr && square->getType() == "SPAWN")
            {
                square->spawnNextItem();
            }
        }
    }
}

bool Board::getActive_() const
{
    return active_;
}

void Board::setActive_(bool active)
{
    active_ = active;
}

King* Board::getKing(PlayerId owner)
{
    for(int r = 0; r < 8; ++r)
    {
        for(int c = 0; c < 8; ++c)
        {
            Square* square = grid_[r][c].get();
            if(square != nullptr)
            {
                Piece* piece = square->getPiece();
                if(piece != nullptr && piece->getType() == PieceType::KING && piece->getOwner() == owner)
                {
                    return dynamic_cast<King*>(piece);
                }
            }
        }
    }
    return nullptr;
}

void Board::print(PlayerId perspective, int turn, int max_turns, int white_mana, int black_mana, int max_mana, bool can_pass)
{
    std::cout << "Turn " << turn << " / " << max_turns << "\n\n";
    if (perspective == PlayerId::WHITE)
        std::cout << "Black mana: " << black_mana << "/" << max_mana << "\n\n";
    else
        std::cout << "White mana: " << white_mana << "/" << max_mana << "\n\n";

    // Rows
    for (int r = 0; r < 8; ++r) {
        int rank = (perspective == PlayerId::WHITE) ? 8 - r : r + 1;
        int row = (perspective == PlayerId::WHITE) ? 7 - r : r;
        std::cout << rank << " ";
        for (int c = 0; c < 8; ++c) {
            int file = (perspective == PlayerId::WHITE) ? c : 7 - c;
            Square* sq = grid_[row][file].get();

            // Background color
            std::string square_type = sq->getType();
            if(square_type == "MANA")
            {
              std::cout << BG_MANA;
            }
            else if(square_type == "BOOST")
            {
              std::cout << BG_BOOST;
            }
            else if(square_type == "SPAWN")
            {
              std::cout << BG_SPAWN;
            }
            else std::cout << (sq->getColor() == SquareColor::BLACK ? BG_DARK : BG_LIGHT);

            // Item and piece. A cloaked enemy piece is hidden from the current perspective.
            Piece* piece = sq->getPiece();
            bool hide_piece = piece != nullptr && piece->hasItem("CLOAK") && piece->getOwner() != perspective;
            std::string item = " ";
            if(piece != nullptr && !hide_piece && piece->getType() == PieceType::KING)
            {
              King* king = dynamic_cast<King*>(piece);
              Player king_player(piece->getOwner());
              bool frightened_capture_restricted = king != nullptr && piece->getPieceId() == "KFRT" &&
                                                    king->inCheck(*this, king_player, turn);
              if(king != nullptr && piece->getOwner() == perspective &&
                 king->inCheckmate(*this, king_player, turn, frightened_capture_restricted))
              {
                item = "#";
              }
              else if(king != nullptr && piece->getOwner() == perspective &&
                      king->inStalemate(*this, king_player, can_pass, turn, frightened_capture_restricted))
              {
                item = "?";
              }
              else if(king != nullptr && king->inCheck(*this, king_player, turn))
              {
                item = "!";
              }
              else if(piece->hasItem())
              {
                item = piece->getItemDisplayName();
              }
            }
            else if(piece != nullptr && !hide_piece && piece->hasItem())
            {
              item = piece->getItemDisplayName();
            }
            else if(piece == nullptr && sq->hasItem())
            {
              item = sq->getItemDisplayName();
            }
            std::cout << item;

            // Piece
            if (piece != nullptr && !hide_piece)
            {
              std::cout << (piece->getOwner() == PlayerId::WHITE ? FG_WHITE : FG_BLACK);
              std::string abbr = piece->getShortName();
              std::cout << abbr;
              if (abbr.length() <= 4)
              {
              std::cout << " "; // only if name is short
              }
            }
            else 
            {
              std::cout << "   ";
            }
            std::cout << RESET;
        }
        std::cout << "\n";
    }

    // Files
    std::cout << " ";
    if (perspective == PlayerId::WHITE)
        for (char f = 'A'; f <= 'H'; ++f) std::cout << "   " << f;
    else
        for (char f = 'H'; f >= 'A'; --f) std::cout << "   " << f;
    std::cout << "\n\n";
    if (perspective == PlayerId::WHITE)
        std::cout << "White mana: " << white_mana << "/" << max_mana << "\n";
    else
        std::cout << "Black mana: " << black_mana << "/" << max_mana << "\n";
}

void Board::setEnPassantPawn(int file, int rank, PlayerId owner)
{
    en_passant_available_ = true;
    en_passant_file_ = file;
    en_passant_rank_ = rank;
    en_passant_owner_ = owner;
}

void Board::clearEnPassant()
{
    en_passant_available_ = false;
    en_passant_file_ = -1;
    en_passant_rank_ = -1;
}

bool Board::isEnPassantCapture(PlayerId capturer, int start_rank, int start_file,
                               int target_rank, int target_file) const
{
    if(!en_passant_available_) return false;
    if(en_passant_owner_ == capturer) return false;
    const int direction = capturer == PlayerId::WHITE ? 1 : -1;
    if(start_rank != en_passant_rank_) return false;
    if(target_file != en_passant_file_) return false;
    if(target_rank != en_passant_rank_ + direction) return false;
    if(std::abs(target_file - start_file) != 1) return false;
    const Square* target_square = getSquare(target_file, target_rank);
    const Square* captured_square = getSquare(en_passant_file_, en_passant_rank_);
    const Piece* captured_piece = captured_square ? captured_square->getPiece() : nullptr;
    return target_square != nullptr && target_square->getPiece() == nullptr &&
           captured_piece != nullptr && captured_piece->getType() == PieceType::PAWN &&
           captured_piece->getOwner() == en_passant_owner_;
}

Square* Board::getEnPassantCapturedSquare(PlayerId capturer, int start_rank, int start_file,
                                          int target_rank, int target_file)
{
    if(!isEnPassantCapture(capturer, start_rank, start_file, target_rank, target_file)) return nullptr;
    return getSquare(en_passant_file_, en_passant_rank_);
}


Board::UndoRecord Board::makeMoveSimulation(const std::string& command_string, Player& active_player, Player& opponent,
                                           int turn_count, bool frightened_king_cannot_capture)
{
    UndoRecord rec;
    rec.valid = false;

    // Save en-passant
    rec.en_passant_prev_available = en_passant_available_;
    rec.en_passant_prev_file = en_passant_file_;
    rec.en_passant_prev_rank = en_passant_rank_;
    rec.en_passant_prev_owner = en_passant_owner_;

    // Save mana for white/black in a consistent order
    if (active_player.getId() == PlayerId::WHITE)
    {
      rec.white_mana_before = active_player.getMana();
      rec.black_mana_before = opponent.getMana();
    }
    else
    {
      rec.white_mana_before = opponent.getMana();
      rec.black_mana_before = active_player.getMana();
    }

    // Snapshot full board: clone any piece present, copy square metadata
    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            Square* sq = getSquare(c, r);
            if (!sq) continue;
            Board::SquareSnapshot &ss = rec.squares[r][c];
            ss.type = sq->getType();
            ss.color = sq->getColor();
            ss.item_id = sq->getItemId();
            Piece* p = sq->getPiece();
            if (p)
            {
                ss.piece = p->clone(); // deep clone
            }
            else
            {
                ss.piece.reset();
            }
        }
    }

    // Snapshot both players' prison contents
    // We don't have direct white_/black_ members in Board, so snapshot using passed players
    // Determine which argument is white/black
    Player* white_ptr = (active_player.getId() == PlayerId::WHITE) ? &active_player : &opponent;
    Player* black_ptr = (white_ptr == &active_player) ? &opponent : &active_player;

    rec.white_prison_before.clear();
    for (const auto& p : white_ptr->getPrison())
    {
      if (p) rec.white_prison_before.push_back(p->clone());
    }
    rec.black_prison_before.clear();
    for (const auto& p : black_ptr->getPrison())
    {
      if (p) rec.black_prison_before.push_back(p->clone());
    }

    // Execute the command via Command::executeCommand on this live board
    std::vector<std::string> tokens;
    Utils::tokenize(command_string, tokens, ' ');
    if (tokens.empty()) return rec;

    // Temporarily set active player to AI to avoid interactive prompts
    bool prev_ai = active_player.isAI();
    active_player.setIsAI(true);

    CommandLine sim_cl;
    bool turn_ended = false;
    Command cmd(tokens);
    bool exec_ok = cmd.executeCommand(sim_cl, turn_ended, *this, active_player, nullptr, turn_count, frightened_king_cannot_capture);

    // restore AI flag
    active_player.setIsAI(prev_ai);

    if (!exec_ok)
    {
      
      return rec;
    }

    rec.valid = true;
    return rec;
}

void Board::undoMoveSimulation(UndoRecord& rec, Player& active_player, Player& opponent)
{
    if (!rec.valid) return;

    // Restore en-passant
    if (rec.en_passant_prev_available)
    {
      setEnPassantPawn(rec.en_passant_prev_file, rec.en_passant_prev_rank, rec.en_passant_prev_owner);
    }
    else
    {
      clearEnPassant();
    }

    // Restore mana for white/black
    Player* white_ptr = (active_player.getId() == PlayerId::WHITE) ? &active_player : &opponent;
    Player* black_ptr = (white_ptr == &active_player) ? &opponent : &active_player;

    white_ptr->setMana(rec.white_mana_before);
    black_ptr->setMana(rec.black_mana_before);

    // Restore full board: for each square, remove any current piece and move in the cloned piece from the snapshot.
    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            Square* sq = getSquare(c, r);
            if (!sq) continue;

            // remove current piece
            std::unique_ptr<Piece> cur = sq->releasePiece();
            (void)cur; // discard

            // restore piece if snapshot has one
            if (rec.squares[r][c].piece)
            {
              std::unique_ptr<Piece> piece_to_restore = std::move(rec.squares[r][c].piece);
              if (piece_to_restore)
              {
                piece_to_restore->setCoordinates(Coordinates(c, r));
                piece_to_restore->initializeOnSquare(sq);
                sq->setPiece(std::move(piece_to_restore));
              }
            }

            // restore square metadata
            sq->setType(rec.squares[r][c].type);
            sq->setColor(rec.squares[r][c].color);
            sq->setItem(rec.squares[r][c].item_id);
        }
    }

    // Restore prisons completely
    white_ptr->restorePrison(std::move(rec.white_prison_before));
    black_ptr->restorePrison(std::move(rec.black_prison_before));

    
    rec.valid = false;
}