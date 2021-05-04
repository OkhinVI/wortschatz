#ifndef SERIALIZESTRING_H
#define SERIALIZESTRING_H

#include <string>
#include <iostream>
#include <limits>
#include <stdexcept>
#include "utility.h"

// serialize and deserialize

class SerializeWrapperInterfase
{
public:
    virtual std::string serialize() = 0;
    virtual void deserialize(std::string &_str) = 0;
    virtual ~SerializeWrapperInterfase() {}
};

class SerializeWrapperString: public SerializeWrapperInterfase
{
public:
    SerializeWrapperString(std::string& _str): str(_str) {}
    virtual ~SerializeWrapperString() override {}
    virtual std::string serialize() override
    {
        return str;
    }
    virtual void deserialize(std::string &_str) override
    {
        str = substrWithoutSideSpaces(_str);
    }
private:
    std::string &str;
};

template<class unsignedT>
static inline void StdStringToUnsignedNum(const std::string &str, unsignedT &num)
{
    const unsigned long long numLong = std::stoull(str);
    if (numLong > std::numeric_limits<unsignedT>::max())
        throw std::out_of_range ("argument exceeds the range");
    else
        num = numLong;

}

template<class signedT>
static inline void StdStringToSignedNum(const std::string &str, signedT &num)
{
    const long long numLong = std::stoull(str);
    if (numLong > std::numeric_limits<signedT>::max())
        throw std::out_of_range ("argument exceeds the range");
    else if (numLong < std::numeric_limits<signedT>::min())
        throw std::out_of_range ("argument less the range");
    else
        num = numLong;
}

static inline void StdStringToNum(const std::string &str, signed char &num) { StdStringToSignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, short int &num) { StdStringToSignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, int &num) { StdStringToSignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, long &num) { StdStringToSignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, long long &num) { StdStringToSignedNum(str, num); }

static inline void StdStringToNum(const std::string &str, bool &num) { StdStringToUnsignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, unsigned char &num) { StdStringToUnsignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, unsigned short &num) { StdStringToUnsignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, unsigned int &num) { StdStringToUnsignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, unsigned long &num) { StdStringToUnsignedNum(str, num); }
static inline void StdStringToNum(const std::string &str, unsigned long long &num) { StdStringToUnsignedNum(str, num); }


template<class T>
class SerializeWrapperNum: public SerializeWrapperInterfase
{
public:
    SerializeWrapperNum(T &_num): num(_num) {}
    virtual ~SerializeWrapperNum() override {}
    virtual std::string serialize() override
    {
        return std::to_string(num);
    }
    virtual void deserialize(std::string &_str) override
    {
        try {
            StdStringToNum(_str, num);
        }
        catch(std::invalid_argument &) {
             // TODO: check error
        }
        catch(std::out_of_range &) {
             // TODO: check error
        }
    }
private:
    T &num;
};

template<class T>
class SerializeWrapperEnum: public SerializeWrapperInterfase
{
public:
    explicit SerializeWrapperEnum(T &_en): en(_en) {}
    virtual ~SerializeWrapperEnum() override {}
    void fromInt(const int val) {
        if (val < static_cast<int>(T::None))
            throw std::out_of_range ("argument less the range");
        else if (val >= static_cast<int>(T::_last_one))
            throw std::out_of_range ("argument exceeds the range");
        else
            en = static_cast<T>(val);
    }
    virtual std::string serialize() override
    {
        return std::to_string(static_cast<int>(en));
    }
    virtual void deserialize(std::string &_str) override
    {
        try {
            fromInt(std::stoi(_str)); // TODO: check error
        }
        catch(std::invalid_argument &) {
             // TODO: check error
        }
        catch(std::out_of_range &) {
             // TODO: check error
        }
    }
private:
    T &en;
};




#endif // SERIALIZESTRING_H
