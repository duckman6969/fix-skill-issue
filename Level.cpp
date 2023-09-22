#pragma once
#include <iostream>
#include "Memory.cpp"
#include "Offsets.cpp"

class Level
{
public:
    long getBasePointer()
    {
        long basePointer = offsets::REGION + offsets::LEVEL;
        return basePointer;
    }
    std::string getName()
    {
        long basePointer = getBasePointer();
        std::string result = mem::ReadString(basePointer);
        return result;
    }
    bool isPlayable()
    {
        auto name = getName();
        if (name.empty())
            return false;
        if (name.compare("mp_lobby") == 0)
            return false;
        return true;
    }
    bool isTrainingArea()
    {
        auto name = getName();
        if (name.compare("mp_rr_canyonlands_staging_mu1") == 0)
            return true;
        return false;
    }
    void print()
    {
        std::string str;
        str += "Level:\n";
        str += "\tBasePointer:\t\t\t\t" + mem::convertPointerToHexString(getBasePointer()) + "\n";
        str += "\tName:\t\t\t\t\t" + getName() + "\n";
        std::cout << str;
    }
};
