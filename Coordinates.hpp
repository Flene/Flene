//----------------------------------------------------------------------------------------------------------------------
/// The Coordinates class represents and validates standard board positions, handling 
/// conversions between string notations (e.g., "a1") and internal grid indices.
///
/// Author(s): 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------

#ifndef COORDINATES_HPP
#define COORDINATES_HPP

#include <string>

class Coordinates
{
  private:
    int file_;
    int rank_;

  public:
    Coordinates(int file_, int rank_);
    Coordinates(const std::string& coordinate_str);

    Coordinates(const Coordinates& other) = default;
    ~Coordinates() = default;

    int getFile() const;
    int getRank() const;

    void setFile(int file_);
    void setRank(int rank_);

    static bool isValid(const std::string& coordinate_str);
    std::string toString() const;

    // Operator overloading
    bool operator==(const Coordinates& other) const noexcept
    {
      return file_ == other.file_ && rank_ == other.rank_;
    }
    bool operator!=(const Coordinates& other) const noexcept
    {
      return !(*this == other);
    }
};

#endif