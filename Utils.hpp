//----------------------------------------------------------------------------------------------------------------------
/// The Utils class contains some useful functions for use in other classes. These include type conversions, 
/// trimming input, converting the case of strings as well as splitting a string into tokens.
///
/// Author(s): Tutors, 12514109, 12312471, 12505788
//----------------------------------------------------------------------------------------------------------------------
#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <format>

class Utils
{
  public:
    //------------------------------------------------------------------------------------------------------------------
    /// @brief Constructor is deleted explicitly.
    Utils() = delete;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief Copy constructor is deleted explicitly.
    Utils(const Utils &) = delete;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief Destructor is deleted explicitly.
    virtual ~Utils() = delete;

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function converts a string into an int. The conversion is only successful, if all
    ///        elements of the string are converted.
    /// @param string string that should be converted
    /// @param out the converted int
    /// @return true, if conversion was successful, false otherwise
    static bool stringToInt(const std::string &string, int &out);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function converts a string into an std::size_t. The conversion is only successful, if all
    ///        elements of the string are converted.
    /// @param string string that should be converted
    /// @param out the converted std::size_t
    /// @return true, if conversion was successful, false otherwise
    static bool stringToSizeT(const std::string &string, std::size_t &out);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function converts a string into a float. The conversion is only successful, if all
    ///        elements of the string are converted.
    /// @param string string that should be converted
    /// @param out the converted float
    /// @return true, if conversion was successful, false otherwise
    static bool stringToFloat(const std::string &string, float &out);


    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function converts a string into a double. The conversion is only successful, if all
    ///        elements of the string are converted.
    /// @param string string that should be converted
    /// @param out the converted double
    /// @return true, if conversion was successful, false otherwise
    static bool stringToDouble(const std::string &string, double &out);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function removes leading whitespaces from a string.
    /// @param string the string to remove leading whitespaces from
    static void trimStart(std::string &string);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function removes trailing whitespaces from a string.
    /// @param string the string to remove trailing whitespaces from
    static void trimEnd(std::string &string);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function removes trailing and leading whitespaces from a string.
    /// @param string the string to remove whitespaces from
    static void trim(std::string &string);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function converts a string to lower case.
    /// @param string the string to convert
    static void toLowerCase(std::string &string);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function converts a string to upper case.
    /// @param string the string to convert
    static void toUpperCase(std::string &string);

    //------------------------------------------------------------------------------------------------------------------
    /// @brief This function splits a string into tokens using a delimiter and stores the tokens in a vector.
    /// @param string the string to split
    /// @param tokens the vector to store the tokens in
    /// @param delimiter delimiter to use for splitting
    static void tokenize(const std::string &string, std::vector<std::string> &tokens, char delimiter);
};

#endif
