#include <cstdint>
#include <cstring>
#include "../Lib_Croix/CroixPharma.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>

CroixPharma croix;
uint8_t bitmap[SIZE][SIZE];
std::vector<std::string> lines;

std::vector<std::string> pharmaFiles;

std::size_t fileIndex = 0;

void displayImage()
{
    memset(bitmap, 0, sizeof(bitmap));

    for (std::size_t y = 0; y < lines.size(); y++) {
        for (std::size_t x = 0; x < lines[y].length(); x++) {
            if (lines[y][x] == 'x')
                bitmap[y][x] = 1;
        }
    }

    croix.writeBitmap(bitmap);
}

// In case not compiled with C++20
bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void getAllPharmaFiles()
{
    const std::string path = "VideoPlayer/output/";
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (hasEnding(entry.path().string(), ".pharma")) {
            pharmaFiles.push_back(entry.path().string());
        }
    }
    auto compare = [](const std::string &a, const std::string &b) {
        std::regex rgx("[0-9]+");
        std::smatch match_a;
        std::smatch match_b;
        std::string a_nb, b_nb;

        if (std::regex_search(a.begin(), a.end(), match_a, rgx))
            a_nb = match_a[0];
        if (std::regex_search(b.begin(), b.end(), match_b, rgx))
            b_nb = match_b[0];
        return std::stoi(a_nb) < std::stoi(b_nb);
    };
    std::sort(pharmaFiles.begin(), pharmaFiles.end(), compare);
}

int main() {
    if (wiringPiSetupGpio() < 0) {
        return 1;
    }

    croix.begin();
    croix.setSide(CroixPharma::BOTH);
    getAllPharmaFiles();

    while (true) {
        lines.clear();

        std::ifstream file(pharmaFiles[fileIndex % pharmaFiles.size()]);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        fileIndex++;

        displayImage();
        delay(100);
    }
    
    return 0;
}
