#pragma once
#include <vector>
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "Math.cpp"
#include "Level.cpp"
#include "X11Utils.cpp"
#include "Aimbot.cpp"

struct ScanResult
{
    int spectators = 0;
    int minDistance = 0;
    std::array<std::array<int, 9>, 9> enemiesMap {};
    int targetHP = 0;
};

static const std::array<int, 8> Distances { -200, -100, -50, -5, 5, 50, 100, 200 };

class Radar
{
private:
    ConfigLoader *m_configLoader;
    Level *m_level;
    LocalPlayer *m_localPlayer;
    RemotePlayers *m_remotePlayers;
    Aimbot *m_aimbot;
    X11Utils *m_x11Utils;

    int m_lastLockedOnIndex = -1;

public:
    Radar(ConfigLoader *configLoader,
             Level *level,
             LocalPlayer *localPlayer,
             RemotePlayers *remotePlayers,
             X11Utils *x11Utils,
             Aimbot *aimbot)
    {
        m_configLoader = configLoader;
        m_level = level;
        m_localPlayer = localPlayer;
        m_remotePlayers = remotePlayers;
        m_x11Utils = x11Utils;
        m_aimbot = aimbot;
    }

    void update()
    {
        auto& players = m_remotePlayers->getPlayers();
        for (std::size_t i = 0; i < players.size(); i++)
        {
            Player *player = &players.at(i);
            player->checkSpectator(m_localPlayer->getPitch(), m_localPlayer->getYaw());
        }
    }

    ScanResult scan(const char* nowStr)
    {
        ScanResult result {};
        if (!m_localPlayer->isValid())
            return result;
        int gameMode = m_configLoader->getGameMode();
        auto& players = m_remotePlayers->getPlayers();
        int lockedOnIndex = m_aimbot->getLockedOnIndex();
        double localX = m_localPlayer->getLocationX();
        double localY = m_localPlayer->getLocationY();
        double yaw = m_localPlayer->getYaw();
        result.minDistance = 999;
        //printf("yaw %lf\n", yaw);
        for (std::size_t i = 0; i < players.size(); i++)
        {
            Player *player = &players.at(i);
            result.spectators += player->isSpectator();
            if (player->isValid() && !player->isKnocked() &&
                !player->isSameTeam(m_localPlayer, gameMode)) {
                double targetX = player->getLocationX();
                double targetY = player->getLocationY();
                double distance = math::distanceToMeters(
                    math::calculateDistance2D(localX, localY, targetX, targetY));
                double diffX_ = math::distanceToMeters(targetX - localX);
                double diffY_ = math::distanceToMeters(targetY - localY);
                double diffX = 0;
                double diffY = 0;
                math::convertPointByYaw(yaw, diffX_, diffY_, diffX, diffY);
                int indexX = Distances.size();
                int indexY = Distances.size();
                for (std::size_t j = 0; j < Distances.size(); ++j) {
                    if (diffX < Distances[j]) {
                        indexX = j;
                        break;
                    }
                }
                for (std::size_t j = 0; j < Distances.size(); ++j) {
                    if (-diffY < Distances[j]) {
                        indexY = j;
                        break;
                    }
                }
                result.enemiesMap[indexY][indexX] += 1;
                result.minDistance = std::min<int>(result.minDistance, distance);
            }
        }
        if (lockedOnIndex >= 0 &&
            static_cast<std::size_t>(lockedOnIndex) < players.size())
            m_lastLockedOnIndex = lockedOnIndex;
        if (m_lastLockedOnIndex >= 0 &&
            static_cast<std::size_t>(m_lastLockedOnIndex) < players.size()) {
            auto& player = players.at(m_lastLockedOnIndex);
            if (player.isValid())
                result.targetHP = player.getShieldValue() + player.getHealthValue();
            else
                m_lastLockedOnIndex = -1;
        }
        utils::clearScreen();
        printf("[%s] Spectators: %d, Min Distance: %d, Target HP: %d, Radar:\n",
            nowStr,
            result.spectators,
            result.minDistance,
            result.targetHP);
        for (size_t i = 0; i < result.enemiesMap.size(); ++i) {
            const auto& row = result.enemiesMap[i];
            printf("[ ");
            for (size_t j = 0; j < row.size(); ++j) {
                bool isClose = (i >= 2 && i < 7 && j >= 2 && j < 7);
                int count = row[j];
                if (isClose && count >= 1) {
                    printf("\033[0;31m%2d\033[0m ", count);
                } else if (isClose) {
                    printf("\033[0;33m%2d\033[0m ", count);
                } else {
                    printf("%2d ", count);
                }
            }
            printf("]\n");
        }
        return result;
    }
};
