#include <cstdint>
#include <cstring>
#include "../Lib_Croix/CroixPharma.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

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
    const std::string path = "output/";
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (hasEnding(entry.path().string(), ".pharma")) {
            pharmaFiles.push_back(entry.path().string());
        }
    }
}

int main() {
    if (wiringPiSetupGpio() < 0) {
        return 1;
    }

    croix.begin();
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
