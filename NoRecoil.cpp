#pragma once
#include <vector>
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "Math.cpp"
#include "Level.cpp"
#include "X11Utils.cpp"

class NoRecoil
{
private:
    ConfigLoader *m_configLoader;
    Level *m_level;
    LocalPlayer *m_localPlayer;

    double m_previousPunchPitch = 0;
    double m_previousPunchYaw = 0;

public:
    NoRecoil(ConfigLoader *configLoader,
             Level *level,
             LocalPlayer *localPlayer)
    {
        m_configLoader = configLoader;
        m_level = level;
        m_localPlayer = localPlayer;
    }
    void update()
    {
        // validations
        if (m_localPlayer->isDead())
            return;
        if (m_localPlayer->isKnocked())
            return;

        // adjust pitch
        const double norecoilPitchStrength = m_configLoader->getNorecoilPitchStrength();
        const double punchPitch = m_localPlayer->getPunchPitch();
        if (punchPitch != 0 && norecoilPitchStrength >= 0.01)
        {
            const double pitch = m_localPlayer->getPitch();
            const double punchPitchDelta = (punchPitch - m_previousPunchPitch) * norecoilPitchStrength;
            m_localPlayer->setPitch(pitch - punchPitchDelta);
            m_previousPunchPitch = punchPitch;
        }

        // adjust yaw
        const double norecoilYawStrength = m_configLoader->getNorecoilYawStrength();
        const double punchYaw = m_localPlayer->getPunchYaw();
        if (punchYaw != 0 && norecoilYawStrength >= 0.01)
        {
            const double yaw = m_localPlayer->getYaw();
            const double punchYawDelta = (punchYaw - m_previousPunchYaw) * norecoilYawStrength;
            m_localPlayer->setYaw(yaw - punchYawDelta);
            m_previousPunchYaw = punchYaw;
        }
    }
};
