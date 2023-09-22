#pragma once
#include <iostream>
#include <sstream>
#include <sys/uio.h>
#include <math.h>
#include <algorithm>
#include <cctype>
#include <locale>
#include <iterator>
#include <chrono>
#include <thread>
#include <random>

static std::minstd_rand RandomGenerator { std::random_device()() };

namespace utils
{
    template <typename T>
    std::string convertNumberToString(const T a_value)
    {
        std::ostringstream out;
        out.precision(6);
        out << std::fixed << a_value;
        return out.str();
    }

    // trim from start (in place)
    static inline void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                             { return !std::isspace(ch); })
                    .base(),
                s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s)
    {
        ltrim(s);
        rtrim(s);
    }

    std::vector<std::string> static inline split(std::string s)
    {
        std::stringstream ss(s);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> tokens(begin, end);
        return tokens;
    }

    bool toBool(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::istringstream is(str);
        bool b;
        is >> std::boolalpha >> b;
        return b;
    }

    int randomInt(int minValue, int maxValue) {
        return std::uniform_int_distribution<int>(minValue, maxValue)(RandomGenerator);
    }

    float randomFloat(float minValue, float maxValue) {
        return std::uniform_real_distribution<float>(minValue, maxValue)(RandomGenerator);
    }

    void randomSleep(int minSleep, int maxSleep) {
        std::this_thread::sleep_for(std::chrono::milliseconds(randomInt(minSleep, maxSleep)));
    }

    void clearScreen() {
        printf("\e[H\e[2J\e[3J");
    }
}
