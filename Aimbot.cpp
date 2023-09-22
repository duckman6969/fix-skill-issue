#pragma once
#include "math.h"
#include <vector>
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "X11Utils.cpp"
#include "Math.cpp"

// Change it if you use other button
static const int SWITCH_WEAPON_BUTTON = 7;

class Aimbot
{
private:
    ConfigLoader *m_configLoader;
    Level *m_level;
    LocalPlayer *m_localPlayer;
    RemotePlayers *m_remotePlayers;
    X11Utils *m_x11Utils;

    int m_lockedOnIndex = -1;
    int m_lastFireIndex = -1;
    std::chrono::steady_clock::time_point m_lockedOnTime = {};
    std::chrono::steady_clock::time_point m_lastFireTime = {};
    std::chrono::steady_clock::time_point m_reloadTime = {};
    std::chrono::steady_clock::time_point m_switchTime = {};

    double m_lastX = 0;
    double m_lastY = 0;
    double m_lastZ = 0;
    std::chrono::steady_clock::time_point m_lastPositionUpdated = {};

public:
    Aimbot(ConfigLoader *configLoader,
           Level *level,
           LocalPlayer *localPlayer,
           RemotePlayers *remotePlayers,
           X11Utils *x11Utils)
    {
        m_configLoader = configLoader;
        m_level = level;
        m_localPlayer = localPlayer;
        m_remotePlayers = remotePlayers;
        m_x11Utils = x11Utils;
    }
    int getLockedOnIndex()
    {
        return m_lockedOnIndex;
    }
    void update()
    {
        // validations
        bool enable = false;
        bool isKeyDown = false;
        if (m_configLoader->getAimbotTrigger() != 0x0000 &&
            m_x11Utils->isKeyDown(m_configLoader->getAimbotTrigger())) {
            enable = true;
            isKeyDown = true;
        }
        if (m_localPlayer->isInAttack()) // will affect nades
            enable = true;
        // if (m_localPlayer->isZooming())
        //    enable = true;
        if (!m_level->isPlayable())
            enable = false;
        if (m_localPlayer->isKnocked() || m_localPlayer->isDead())
            enable = false;
        if (!enable)
        {
            unlockTarget();
            m_x11Utils->mouseEvent(ButtonRelease, 1, 0, true);
            return;
        }

        // find target
        auto& players = m_remotePlayers->getPlayers();
        Player* lockedOnPlayer = nullptr;
        if (m_lockedOnIndex >= 0 &&
            static_cast<std::size_t>(m_lockedOnIndex) < players.size())
            lockedOnPlayer = &players.at(m_lockedOnIndex);
        if (lockedOnPlayer == nullptr ||
            !lockedOnPlayer->isValid() ||
            !lockedOnPlayer->isVisible() ||
            lockedOnPlayer->isKnocked() ||
            lockedOnPlayer->isDead()) {
            lockedOnPlayer = nullptr;
            findClosestEnemy(players);
            if (m_lockedOnIndex >= 0)
                lockedOnPlayer = &players.at(m_lockedOnIndex);
        }
        if (lockedOnPlayer == nullptr) {
            m_x11Utils->mouseEvent(ButtonRelease, 1, 0, true);
            return;
        }

        // predict position
        double localX = m_localPlayer->getLocationX();
        double localY = m_localPlayer->getLocationY();
        double localZ = m_localPlayer->getLocationZ();
        double targetX = lockedOnPlayer->getLocationX();
        double targetY = lockedOnPlayer->getLocationY();
        double targetZ = lockedOnPlayer->getLocationZ();
        double distanceToTarget = math::calculateDistanceInMeters(
            localX, localY, localZ, targetX, targetY, targetZ);
        if (distanceToTarget > m_configLoader->getAimbotMaxRange()) {
            m_x11Utils->mouseEvent(ButtonRelease, 1);
            return;
        }
        auto now = std::chrono::steady_clock::now();
        WeaponInfo info = m_localPlayer->getWeaponInfo();
        double timeToTarget = distanceToTarget / info.speed * 1000;
        if (m_lastX != 0 && m_lastY != 0 && m_lastZ != 0) {
            std::chrono::duration<double, std::milli> duration = now - m_lastPositionUpdated;
            double diffX = (targetX - m_lastX) / duration.count();
            double diffY = (targetY - m_lastY) / duration.count();
            double diffZ = (targetZ - m_lastZ) / duration.count();
            /*printf("predict (%f, %f, %f) -> (%f %f %f) | %f %f %f\n",
                m_lastX, m_lastY, m_lastZ, targetX, targetY, targetZ,
                diffX * timeToTarget, diffY * timeToTarget, diffZ * timeToTarget);*/
            if (duration.count() >= 100) {
                m_lastX = targetX;
                m_lastY = targetY;
                m_lastZ = targetZ;
                m_lastPositionUpdated = now;
            }
            targetX += diffX * timeToTarget;
            targetY += diffY * timeToTarget;
            targetZ += diffZ * timeToTarget;
        } else {
            m_lastX = targetX;
            m_lastY = targetY;
            m_lastZ = targetZ;
            m_lastPositionUpdated = now;
        }

        // get desired angle to an enemy
        double desiredViewAngleYaw = math::calculateDesiredYaw(
            localX, localY, targetX, targetY);
        double desiredViewAnglePitch = math::calculateDesiredPitch(
            localX, localY, localZ, targetX, targetY, targetZ);
        bool isCrosshair = lockedOnPlayer->isCrosshair();

        // Check is close
        bool isFar = distanceToTarget > 80;
        bool isClose = distanceToTarget <= 50;
        bool isVeryClose = distanceToTarget <= 30;
        bool isVeryVeryClose = distanceToTarget <= 10;
        double activationFovH = m_configLoader->getAimbotActivationFOV();
        double activationFovV = activationFovH * 3;
        double limit = m_configLoader->getAimbotActivationFOV() * 1.0 / m_configLoader->getAimbotSmoothing();
        double triggerFovH = 0.05;
        double triggerFovV = 0.10;
        if (isVeryVeryClose) {
            activationFovH *= 3;
            activationFovV *= 3;
            triggerFovH = 0.4;
            triggerFovV = 0.6;
            limit *= 2;
        } else if (isVeryClose) {
            activationFovH *= 2;
            activationFovV *= 2;
            triggerFovH = 0.2;
            triggerFovV = 0.4;
        } else if (isClose) {
            triggerFovH = 0.1;
            triggerFovV = 0.2;
        } else if (isFar) {
            triggerFovH = 0.03;
            triggerFovV = 0.03;
        }
        if (m_lastFireIndex == m_lockedOnIndex) {
            activationFovH *= 2;
            activationFovV *= 2;
        }
        if (m_lastFireIndex == m_lockedOnIndex) {
            limit *= 2;
        }

        // Setup Pitch
        double pitch = m_localPlayer->getPitch();
        double pitchAngleDelta = math::calculatePitchAngleDelta(pitch, desiredViewAnglePitch);
        double pitchAngleDeltaAbs = abs(pitchAngleDelta);
        if (pitchAngleDeltaAbs > activationFovV) {
            unlockTarget();
            m_x11Utils->mouseEvent(ButtonRelease, 1);
            return;
        }

        // Setup Yaw
        double yaw = m_localPlayer->getYaw();
        double angleDelta = math::calculateAngleDelta(yaw, desiredViewAngleYaw);
        double angleDeltaAbs = abs(angleDelta);
        if (angleDeltaAbs > activationFovH) {
            unlockTarget();
            m_x11Utils->mouseEvent(ButtonRelease, 1);
            return;
        }

        // Reload
        /*printf("weapon ammo: %d, is semi: %d, speed: %f, timeToTarget: %f\n",
            info.ammo, info.isSemiAuto, info.speed, timeToTarget);*/
        if (info.ammo == 0 && isKeyDown) {
            int health = lockedOnPlayer->getShieldValue() + lockedOnPlayer->getHealthValue();
            if (isClose && health <= 100 &&
                now - m_switchTime > std::chrono::milliseconds(1000)) {
                utils::randomSleep(50, 100);
                m_x11Utils->mouseEvent(ButtonPress, SWITCH_WEAPON_BUTTON);
                m_x11Utils->mouseEvent(ButtonRelease, SWITCH_WEAPON_BUTTON);
                m_switchTime = now;
            } else if (now - m_reloadTime > std::chrono::milliseconds(300)) {
                utils::randomSleep(50, 100);
                m_x11Utils->mouseEvent(ButtonRelease, 1);
                m_x11Utils->mouseEvent(ButtonPress, 1);
                m_x11Utils->mouseEvent(ButtonRelease, 1);
                m_reloadTime = now;
            }
            return;
        }

        // Aim
        if (!isCrosshair) {
            double newYaw = flipYawIfNeeded(yaw + std::clamp(angleDelta, -limit, limit));
            m_localPlayer->setYaw(newYaw);
            if (now - m_lastFireTime > std::chrono::milliseconds(100)) {
                double newPitch = pitch + std::clamp(pitchAngleDelta, -limit, limit);
                m_localPlayer->setPitch(newPitch);
            }
        }

        // Trigger
        bool waiting = (now - m_lockedOnTime <= std::chrono::milliseconds(50));
        bool inRange = isCrosshair || (angleDeltaAbs <= triggerFovH && pitchAngleDeltaAbs <= triggerFovV);
        bool hasAmmo = info.ammo > 0 || info.isSemiAuto;
        if (!waiting && isKeyDown && inRange && hasAmmo) {
            m_lastFireIndex = m_lockedOnIndex;
            m_lastFireTime = now;
            if (info.isSemiAuto) {
                m_x11Utils->mouseEvent(ButtonPress, 1);
                m_x11Utils->mouseEvent(ButtonRelease, 1, utils::randomInt(120, 170));
            } else {
                m_x11Utils->mouseEvent(ButtonPress, 1, utils::randomInt(300, 500));
            }
        }
    }
    double flipYawIfNeeded(double angle)
    {
        double myAngle = angle;
        if (myAngle > 180)
            myAngle = (360 - myAngle) * -1;
        else if (myAngle < -180)
            myAngle = (360 + myAngle);
        return myAngle;
    }
    void findClosestEnemy(std::vector<Player>& players)
    {
        int gameMode = m_configLoader->getGameMode();
        int closestPlayerIndexSoFar = -1;
        double closestPlayerAngleSoFar = std::numeric_limits<double>::infinity();
        double localX = m_localPlayer->getLocationX();
        double localY = m_localPlayer->getLocationY();
        double localZ = m_localPlayer->getLocationZ();
        double yaw = m_localPlayer->getYaw();
        double pitch = m_localPlayer->getPitch();
        for (std::size_t i = 0; i < players.size(); i++)
        {
            Player* player = &players.at(i);
            if (!player->isValid())
                continue;
            if (player->isKnocked() || player->isDead())
                continue;
            if (player->isSameTeam(m_localPlayer, gameMode))
                continue;
            if (!player->isVisible())
                continue;
            double targetX = player->getLocationX();
            double targetY = player->getLocationY();
            double targetZ = player->getLocationZ();
            double desiredViewAngleYaw = math::calculateDesiredYaw(
                localX, localY, targetX, targetY);
            double desiredViewAnglePitch = math::calculateDesiredPitch(
                localX, localY, localZ, targetX, targetY, targetZ);
            double angleDelta = math::calculateAngleDelta(yaw, desiredViewAngleYaw);
            double pitchAngleDelta = math::calculatePitchAngleDelta(pitch, desiredViewAnglePitch);
            double angle = std::sqrt(std::pow(angleDelta, 2) * std::pow(pitchAngleDelta * 2, 2));
            if (closestPlayerIndexSoFar == -1)
            {
                closestPlayerIndexSoFar = i;
                closestPlayerAngleSoFar = angle;
            }
            else
            {
                if (angle < closestPlayerAngleSoFar)
                {
                    closestPlayerIndexSoFar = i;
                    closestPlayerAngleSoFar = angle;
                }
            }
        }
        auto now = std::chrono::steady_clock::now();
        if (closestPlayerIndexSoFar == -1) {
            unlockTarget();
        } else if (closestPlayerIndexSoFar != m_lastFireIndex &&
            now - m_lastFireTime < std::chrono::milliseconds(300)) {
            // delay before switch to other enemy
            unlockTarget();
        } else {
            m_lockedOnIndex = closestPlayerIndexSoFar;
            if (m_lockedOnIndex != m_lastFireIndex) {
                m_lockedOnTime = now;
            }
        }
    }

    void unlockTarget()
    {
        m_lockedOnIndex = -1;
        m_lastX = 0;
        m_lastY = 0;
        m_lastZ = 0;
    }
};
