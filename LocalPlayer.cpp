#pragma once
#include <iostream>
#include "Utils.cpp"
#include "Memory.cpp"
#include "Offsets.cpp"
#include "Math.cpp"

struct WeaponInfo {
    int ammo = 0;
    float speed = 0;
    bool isSemiAuto = false;
};

class LocalPlayer
{
private:
    long m_basePointer = 0;
    long getUnresolvedBasePointer()
    {
        long unresolvedBasePointer = offsets::REGION + offsets::LOCAL_PLAYER;
        return unresolvedBasePointer;
    }
    long getBasePointer()
    {
        if (m_basePointer == 0)
            m_basePointer = mem::Read<long>(getUnresolvedBasePointer());
        return m_basePointer;
    }

public:
    void markForPointerResolution()
    {
        m_basePointer = 0;
    }
    float getLocationX()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::LOCAL_ORIGIN;
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    float getLocationY()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::LOCAL_ORIGIN + sizeof(float);
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    float getLocationZ()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::LOCAL_ORIGIN + (sizeof(float) * 2);
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    int getTeamNumber()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::TEAM_NUMBER;
        int result = mem::Read<int>(ptrLong);
        return result;
    }
    bool isInAttack()
    {
        long ptrLong = offsets::REGION + offsets::IN_ATTACK;
        int result = mem::Read<int>(ptrLong);
        return result > 0;
    }
    std::string getName()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::NAME;
        std::string result = mem::ReadString(ptrLong);
        return result;
    }
    bool isKnocked()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::BLEEDOUT_STATE;
        short result = mem::Read<short>(ptrLong);
        return result > 0;
    }
    bool isDead()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::LIFE_STATE;
        short result = mem::Read<short>(ptrLong);
        return result > 0;
    }
    bool isValid()
    {
        return getBasePointer() > 0 && !isDead();
    }
    float getPunchPitch()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::VEC_PUNCH_WEAPON_ANGLE;
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    float getPunchYaw()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::VEC_PUNCH_WEAPON_ANGLE + sizeof(float);
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    float getPitch()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::VIEW_ANGLE;
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    void setPitch(float angle)
    {
        if (angle > 90 || angle < -90)
            return;
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::VIEW_ANGLE;
        mem::Write<float>(ptrLong, angle);
    }
    float getYaw()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::VIEW_ANGLE + sizeof(float);
        float result = mem::Read<float>(ptrLong);
        return result;
    }
    void setYaw(float angle)
    {
        if (angle > 180 || angle < -180)
            return;
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::VIEW_ANGLE + sizeof(float);
        mem::Write<float>(ptrLong, angle);
    }
    bool isZooming()
    {
        long basePointer = getBasePointer();
        long ptrLong = basePointer + offsets::ZOOMING;
        short result = mem::Read<short>(ptrLong);
        return result > 0;
    }
    WeaponInfo getWeaponInfo()
    {
        WeaponInfo info;
        long basePointer = getBasePointer();
        long weaponId = mem::Read<long>(basePointer + offsets::OFFSET_WEAPON) & 0xFFFF;
        long weaponEntity = mem::Read<long>(offsets::REGION + offsets::ENTITY_LIST + (weaponId << 5));
        info.ammo = mem::Read<int>(weaponEntity + offsets::OFFSET_WEAPON_AMMO);
        info.speed = math::distanceToMeters(mem::Read<float>(weaponEntity + offsets::OFFSET_WEAPON_SPEED));
        info.isSemiAuto = mem::Read<int>(weaponEntity + offsets::OFFSET_WEAPON_SEMIAUTO);
        return info;
    }
};
