#pragma once
#include <vector>
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "Math.cpp"
#include "Level.cpp"
#include "X11Utils.cpp"
#include "ConfigLoader.cpp"

class Sense
{
private:
    ConfigLoader *m_configLoader;
    Level *m_level;
    LocalPlayer *m_localPlayer;
    RemotePlayers *m_remotePlayers;

    std::chrono::steady_clock::time_point m_lastUpdated = {};

public:
    Sense(ConfigLoader *configLoader,
          Level *level,
          LocalPlayer *localPlayer,
          RemotePlayers *remotePlayers)
    {
        m_configLoader = configLoader;
        m_level = level;
        m_localPlayer = localPlayer;
        m_remotePlayers = remotePlayers;
    }
    void update()
    {
        auto now = std::chrono::steady_clock::now();
        if (now - m_lastUpdated < std::chrono::milliseconds(10)) {
            return;
        }
        m_lastUpdated = now;
        int senseMaxRange = m_configLoader->getSenseMaxRange();
        int senseMaxRangeOverWall = m_configLoader->getSenseMaxRangeOverWall();
        int gameMode = m_configLoader->getGameMode();
        double localX = m_localPlayer->getLocationX();
        double localY = m_localPlayer->getLocationY();
        double localZ = m_localPlayer->getLocationZ();
        auto& players = m_remotePlayers->getPlayers();
        for (std::size_t i = 0; i < players.size(); i++) {
            Player *player = &players.at(i);
            if (!player->isValid())
                continue;
            if (player->isSameTeam(m_localPlayer, gameMode)) {
                player->setGlowEnable(1);
                player->setGlowThroughWall(1);
                player->setCustomGlow(0, true, true);
                continue;
            }
            double distance = math::calculateDistanceInMeters(
                localX, localY, localZ,
                player->getLocationX(),
                player->getLocationY(),
                player->getLocationZ());
            if (player->isVisible() && !player->isKnocked() && distance < senseMaxRange) {
                player->setGlowEnable(1);
                player->setGlowThroughWall(1);
                int health = player->getShieldValue() + player->getHealthValue();
                player->setCustomGlow(health, true, false);
            } else if (distance < senseMaxRangeOverWall) {
                player->setGlowEnable(1);
                player->setGlowThroughWall(1);
                player->setCustomGlow(0, false, false);
            } else if (player->getGlowEnable() == 1 && player->getGlowThroughWall() == 1) {
                player->setGlowEnable(0);
                player->setGlowThroughWall(0);
            }
        }
    }
};
