#ifndef UTILITY_H
#define UTILITY_H

#include <string>

// use std::string
typedef std::string::size_type size_type;

std::string substrWithoutSideSpaces(const std::string &str, size_type posBegin = 0, size_type n = std::string::npos);
std::string getPrefix(const std::string &str, char &findedDelimiter);

static inline
bool isWort(const char ch)
{
    const unsigned char chU = ch;
    return (chU > 127 || chU == '-' || ::isalpha(chU));
}

// templates with a variable number of arguments

template<typename ... Types>
size_t NumberOfFunctionArguments(Types&... args)
{
    return  sizeof...(args);
}

#endif // UTILITY_H
