#include "Settings.h"
#include <fstream>
#include <string>
#include <filesystem>

namespace Settings
{
    int32_t RecruitmentBaseCost = 500;
    int32_t RecruitmentLevelMultiplier = 50;
    int32_t WeeklyWage = 150;
    int32_t GracePeriodDuration = 30;

    void Load()
    {
        const std::string path = "Data/SKSE/Plugins/NoFreeService.ini";
        
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories("Data/SKSE/Plugins");
            std::ofstream outFile(path);
            if (outFile.is_open()) {
                outFile << "[General]" << std::endl;
                outFile << "RecruitmentBaseCost=500" << std::endl;
                outFile << "RecruitmentLevelMultiplier=50" << std::endl;
                outFile << "WeeklyWage=150" << std::endl;
                outFile << "GracePeriodDuration=30" << std::endl;
                outFile.close();
            }
            return;
        }

        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[') continue;
            
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                s.erase(s.find_last_not_of(" \t\r\n") + 1);
            };
            trim(key);
            trim(value);

            try {
                if (key == "RecruitmentBaseCost") RecruitmentBaseCost = std::stoi(value);
                else if (key == "RecruitmentLevelMultiplier") RecruitmentLevelMultiplier = std::stoi(value);
                else if (key == "WeeklyWage") WeeklyWage = std::stoi(value);
                else if (key == "GracePeriodDuration") GracePeriodDuration = std::stoi(value);
            } catch (...) {}
        }
        file.close();
    }
}
