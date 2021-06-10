#ifndef UTILITY_H
#define UTILITY_H

#include <cstddef>
#include <vector>
#include <string>

namespace util
{

typedef std::vector<std::string> VectorString;

void VectorFromString(VectorString &vec, const std::string &str);

// templates with a variable number of arguments

template<typename ... Types>
size_t NumberOfFunctionArguments(Types... args)
{
    return  sizeof...(args);
}

void replaceEndLines(std::string &str);

std::string getline(std::istream &is);

} // namespace util

#endif // UTILITY_H
