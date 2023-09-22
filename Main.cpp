#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include "Utils.cpp"
#include "Offsets.cpp"
#include "Level.cpp"
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "Sense.cpp"
#include "NoRecoil.cpp"
#include "Aimbot.cpp"
#include "Radar.cpp"
#include "X11Utils.cpp"
#include "ConfigLoader.cpp"

int main(int argc, char *argv[])
{
    ConfigLoader *configLoader = new ConfigLoader();
    if (getuid())
    {
        printf("Not under root.\n");
        return -1;
    }
    if (mem::GetPID() == 0)
    {
        printf("Process not found.\n");
        return -1;
    }
    bool forceGameMode = false;
    if (argc >= 2) {
        if (argv[1] == std::string("1")) {
            configLoader->setGameMode(1);
            forceGameMode = true;
        } else if (argv[1] == std::string("0")) {
            configLoader->setGameMode(0);
            forceGameMode = true;
        }
    }
    // configLoader->print();
    int counter = 0;
    Level *level = new Level();
    LocalPlayer *localPlayer = new LocalPlayer();
    RemotePlayers *remotePlayers = new RemotePlayers(level, &counter);
    X11Utils *x11Utils = new X11Utils();
    Sense *sense = new Sense(configLoader, level, localPlayer, remotePlayers);
    NoRecoil *noRecoil = new NoRecoil(configLoader, level, localPlayer);
    Aimbot *aimbot = new Aimbot(configLoader, level, localPlayer, remotePlayers, x11Utils);
    Radar *radar = new Radar(configLoader, level, localPlayer, remotePlayers, x11Utils, aimbot);

    // Main loop
    printf("Loaded.\n");
    ScanResult scanResult {};
    std::array<char, 32> nowStr {};
    while (1)
    {
        auto now = std::time(nullptr);
        std::strftime(nowStr.data(), nowStr.size(), "%H:%M:%S", std::localtime(&now));
        try
        {
            if (counter % 1000 == 0)
                configLoader->reloadFile(); // will attempt to reload config if there have been any updates to it

            bool wasNotPlayable = false;
            while (!level->isPlayable()) {
                wasNotPlayable = true;
                utils::clearScreen();
                printf("[%s] Initializing.\n", nowStr.data());
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (wasNotPlayable) {
                printf("[%s] Waiting.\n", nowStr.data());
                std::this_thread::sleep_for(std::chrono::seconds(65));
                printf("[%s] Ready.\n", nowStr.data());
            }

            // resolve pointers
            localPlayer->markForPointerResolution();
            remotePlayers->markForPointerResolution();

            // run features
            if (configLoader->isSenseOn())
                sense->update();
            if (configLoader->isAimbotOn() && scanResult.spectators == 0)
                aimbot->update();
            if (configLoader->isNorecoilOn())
                noRecoil->update();
            radar->update();

            // all ran fine
            if (counter % 200 == 0)
            {
                scanResult = radar->scan(nowStr.data());
                if (scanResult.spectators > 0)
                    x11Utils->mouseEvent(ButtonRelease, 1);
                if (!forceGameMode) {
                    auto name = level->getName();
                    // printf("%s\n", name.c_str());
                    if (name == "mp_rr_party_crasher" ||
                        name == "mp_rr_arena_habitat" ||
                        name == "mp_rr_arena_phase_runner")
                        configLoader->setGameMode(1);
                    else
                        configLoader->setGameMode(0);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        catch (const std::exception& e)
        {
            printf("[%s] Error: %s.\n", nowStr.data(), e.what());
            x11Utils->mouseEvent(ButtonRelease, 1, 0, true);
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        counter++;
        if (counter > 10000000)
            counter = 0;
    }
}
