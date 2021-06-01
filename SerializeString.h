#ifndef SERIALIZESTRING_H
#define SERIALIZESTRING_H

#include <string>
#include <iostream>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <sstream>
#include "string_utf8.h"
#include "utility.h"

//--------------------------- serialize -----------------------------------
template<typename T, typename ... Types>
std::ostream& multiPrintIntoStream(std::ostream &os, const char *, T value)
{
    return os << value;
}

template<typename T, typename ... Types>
std::ostream& multiPrintIntoStream(std::ostream &os, const char *delimiter, T value, const Types&... args)
{
    os << value << delimiter;
    return multiPrintIntoStream(os, delimiter, args...);
}

template<typename ... Types>
std::string multiPrintIntoString(const char *delimiter, const Types&... args)
{
    std::stringstream ss;
    multiPrintIntoStream(ss, delimiter, args...);
    return ss.str();
}

//--------------------------- deserialize ---------------------------------

template<typename T>
size_t multiScanFromStream(std::istream &is, T &value)
{
    is >> value;
    return 1;
}

template<typename T, typename ... Types>
size_t multiScanFromStream(std::istream &is, T &value, Types&... args)
{
    is >> value;
    if (is.eof())
        return 1;
    return multiScanFromStream(is, args...) + 1;
}

template<typename ... Types>
size_t multiScanFromString(const std::string &str, Types&... args)
{
    std::stringstream ss(str);
    return multiScanFromStream(ss, args...);
}

//--------------------------- SerializeWrapper ---------------------------------

class SerializeWrapperString
{
public:
    SerializeWrapperString(std::string& _str): str(_str) {}
    std::string serialize()
    {
        return str;
    }
    void deserialize(const std::string &_str)
    {
        str = AreaUtf8(_str).trim().toString();
    }
private:
    std::string &str;
};

// can throw exceptions: std::invalid_argument and std::out_of_range
template <class unsignedT> static inline
typename std::enable_if<std::is_unsigned<unsignedT>::value, unsignedT>::type
StdStringToNum(const std::string &str, unsignedT &num)
{
    const unsigned long long numLong = std::stoull(str);
    if (numLong > std::numeric_limits<unsignedT>::max())
        throw std::out_of_range ("argument exceeds the range");
    else
        num = numLong;
    return num;
}

// can throw exceptions: std::invalid_argument and std::out_of_range
template <class signedT> static inline
typename std::enable_if<std::is_signed<signedT>::value, signedT>::type
StdStringToNum(const std::string &str, signedT &num)
{
    const long long numLong = std::stoull(str);
    if (numLong > std::numeric_limits<signedT>::max())
        throw std::out_of_range ("argument exceeds the range");
    else if (numLong < std::numeric_limits<signedT>::min())
        throw std::out_of_range ("argument less the range");
    else
        num = numLong;
    return num;
}

// can throw exceptions: std::invalid_argument and std::out_of_range
template<class T>
class SerializeWrapperNum
{
public:
    SerializeWrapperNum(T &_num): num(_num) {}
    std::string serialize()
    {
        return std::to_string(num);
    }
    void deserialize(const std::string &_str)
    {
        StdStringToNum(_str, num);
    }
private:
    T &num;
};

// can throw exceptions: std::invalid_argument and std::out_of_range
template<class T>
class SerializeWrapperEnum
{
public:
    explicit SerializeWrapperEnum(T &_en): en(_en) {}
    void fromInt(const int val) {
        if (val < static_cast<int>(T::None))
            throw std::out_of_range ("argument less the range");
        else if (val >= static_cast<int>(T::_last_one))
            throw std::out_of_range ("argument exceeds the range");
        else
            en = static_cast<T>(val);
    }
    std::string serialize()
    {
        return std::to_string(static_cast<int>(en));
    }
    void deserialize(const std::string &_str)
    {
        fromInt(std::stoi(_str));
    }
private:
    T &en;
};

template<class T>
class SerializeWrapperAny
{
public:
    SerializeWrapperAny(T &_val): val(_val) {}
    std::string serialize()
    {
        return val.serialize();
    }
    void deserialize(const std::string &_str)
    {
        val.deserialize(_str);
    }
private:
    T &val;
};


#endif // SERIALIZESTRING_H
