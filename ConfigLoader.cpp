#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "Utils.cpp"

class ConfigLoader
{
private:
    const std::string m_fileName = "config.ini";
    std::vector<std::string> *lines = new std::vector<std::string>;
    long m_lastTimeFileEdited = 0;

    // internal
    // 0: BR, 1: CONTROL/FREEDM
    int m_gameMode = 0;

    // features
    bool m_featureAimbotOn = false;
    bool m_featureNoRecoilOn = false;
    bool m_featureSenseOn = false;

    // sense
    int m_senseMaxRange = 0;
    int m_senseMaxRangeOverWall = 0;

    // aimbot
    int m_aimbotTrigger = 0x0000;
    int m_aimbotSmoothing = 999999;
    int m_aimbotActivationFOV = 0;
    int m_aimbotMaxRange = 0;

    // norecoil
    double m_norecoilPitchStrength = 0;
    double m_norecoilYawStrength = 0;

    bool loadFileIntoVector()
    {
        struct stat result;
        if (stat(m_fileName.c_str(), &result) == 0)
        {
            long modTime = result.st_mtime;
            bool fileNeedsReload = modTime > m_lastTimeFileEdited;
            m_lastTimeFileEdited = modTime;
            if (!fileNeedsReload)
                return false;
        }

        lines->clear();
        std::string str;
        std::ifstream myFile(m_fileName);
        while (getline(myFile, str))
        {
            utils::trim(str);
            if (str.empty())
                continue;
            if (str.rfind("#", 0) == 0)
                continue;
            lines->push_back(str);
        }
        myFile.close();
        return true;
    }

    void parseLines()
    {
        for (std::size_t i = 0; i < lines->size(); i++)
        {
            std::vector<std::string> lineParts = utils::split(lines->at(i));

            // line key
            std::string lineKey(lineParts.at(0));
            utils::trim(lineKey);
            if (lineKey.empty())
                throw "Failed to parse the config";

            // line value
            std::string lineValue(lineParts.at(1));
            utils::trim(lineValue);
            if (lineValue.empty())
                throw "Failed to parse the config";

            // features
            m_featureAimbotOn = (lineKey.compare("FEATURE_AIMBOT_ON") != 0) ? m_featureAimbotOn : utils::toBool(lineValue);
            m_featureNoRecoilOn = (lineKey.compare("FEATURE_NORECOIL_ON") != 0) ? m_featureNoRecoilOn : utils::toBool(lineValue);
            m_featureSenseOn = (lineKey.compare("FEATURE_SENSE_ON") != 0) ? m_featureSenseOn : utils::toBool(lineValue);
            m_gameMode = (lineKey.compare("FEATURE_GAME_MODE") != 0) ? m_gameMode : stoi(lineValue);
            // sense
            m_senseMaxRange = (lineKey.compare("SENSE_MAX_RANGE") != 0) ? m_senseMaxRange : stoi(lineValue);
            m_senseMaxRangeOverWall = (lineKey.compare("SENSE_MAX_RANGE_OVER_WALL") != 0) ? m_senseMaxRangeOverWall : stoi(lineValue);
            // aimbot
            m_aimbotTrigger = (lineKey.compare("AIMBOT_TRIGGER") != 0) ? m_aimbotTrigger : stoi(lineValue, 0, 16);
            m_aimbotSmoothing = (lineKey.compare("AIMBOT_SMOOTHING") != 0) ? m_aimbotSmoothing : stoi(lineValue);
            m_aimbotActivationFOV = (lineKey.compare("AIMBOT_ACTIVATION_FOV") != 0) ? m_aimbotActivationFOV : stoi(lineValue);
            m_aimbotMaxRange = (lineKey.compare("AIMBOT_MAX_RANGE") != 0) ? m_aimbotMaxRange : stoi(lineValue);
            // norecoil
            m_norecoilPitchStrength = (lineKey.compare("NORECOIL_PITCH_STRENGTH") != 0) ? m_norecoilPitchStrength : stod(lineValue);
            m_norecoilYawStrength = (lineKey.compare("NORECOIL_YAW_STRENGTH") != 0) ? m_norecoilYawStrength : stod(lineValue);
        }
    }

public:
    ConfigLoader()
    {
        reloadFile();
    }

    void reloadFile()
    {
        if (loadFileIntoVector())
            parseLines();
    }

    // internal
    void setGameMode(int gameMode)
    {
        m_gameMode = gameMode;
    }
    int getGameMode()
    {
        return m_gameMode;
    }

    // features
    bool isAimbotOn()
    {
        return m_featureAimbotOn;
    }
    bool isNorecoilOn()
    {
        return m_featureNoRecoilOn;
    }
    bool isSenseOn()
    {
        return m_featureSenseOn;
    }

    // sense
    int getSenseMaxRange()
    {
        return m_senseMaxRange;
    }
    int getSenseMaxRangeOverWall()
    {
        return m_senseMaxRangeOverWall;
    }

    // aimbot
    int getAimbotTrigger()
    {
        return m_aimbotTrigger;
    }
    int getAimbotSmoothing()
    {
        return m_aimbotSmoothing;
    }
    int getAimbotActivationFOV()
    {
        return m_aimbotActivationFOV;
    }
    int getAimbotMaxRange()
    {
        return m_aimbotMaxRange;
    }

    // norecoil
    double getNorecoilPitchStrength()
    {
        return m_norecoilPitchStrength;
    }
    double getNorecoilYawStrength()
    {
        return m_norecoilYawStrength;
    }

    void print()
    {
        printf("\n======================== SETTINGS LOADED ========================\n");

        printf("GAME_MODE \t\t\t%d\n", m_gameMode);
        printf("FEATURE_AIMBOT_ON \t\t%s\n", m_featureAimbotOn ? "true" : "false");
        printf("FEATURE_NORECOIL_ON \t\t%s\n", m_featureNoRecoilOn ? "true" : "false");
        printf("FEATURE_SENSE_ON \t\t%s\n", m_featureSenseOn ? "true" : "false");

        printf("SENSE_MAX_RANGE \t\t%d\n", m_senseMaxRange);
        printf("SENSE_MAX_RANGE_OVER_WALL \t\t%d\n", m_senseMaxRangeOverWall);

        printf("AIMBOT_TRIGGER \t\t\t%d\n", m_aimbotTrigger);
        printf("AIMBOT_SMOOTHING \t\t%d\n", m_aimbotSmoothing);
        printf("AIMBOT_ACTIVATION_FOV \t\t%d\n", m_aimbotActivationFOV);
        printf("AIMBOT_MAX_RANGE \t\t%d\n", m_aimbotMaxRange);

        printf("NORECOIL_PITCH_STRENGTH \t%.6f\n", m_norecoilPitchStrength);
        printf("NORECOIL_YAW_STRENGTH \t\t%.6f\n", m_norecoilYawStrength);

        printf("=================================================================\n\n");
    }
};
