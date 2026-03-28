#include <wiringPi.h>
#include <cstdint>
#include <cstring>
#include <ctime>
#include "CroixPharma.h"

CroixPharma croix;
uint8_t bitmap[SIZE][SIZE];

int lastSecond = -1;

const uint8_t digits[10][15] = {
{1,1,1, 1,0,1, 1,0,1, 1,0,1, 1,1,1},
{0,1,0, 1,1,0, 0,1,0, 0,1,0, 1,1,1},
{1,1,1, 0,0,1, 1,1,1, 1,0,0, 1,1,1},
{1,1,1, 0,0,1, 0,1,1, 0,0,1, 1,1,1},
{1,0,1, 1,0,1, 1,1,1, 0,0,1, 0,0,1},
{1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1},
{1,1,1, 1,0,0, 1,1,1, 1,0,1, 1,1,1},
{1,1,1, 0,0,1, 0,1,0, 1,0,0, 1,0,0},
{1,1,1, 1,0,1, 1,1,1, 1,0,1, 1,1,1},
{1,1,1, 1,0,1, 1,1,1, 0,0,1, 1,1,1}
};

void drawDigit(uint8_t d, int xOffset, int yOffset){
    for (int y=0;y<5;y++)
        for (int x=0;x<3;x++)
            if (digits[d][y*3+x])
                bitmap[yOffset+y][xOffset+x] = 1;
}

void drawColon(int xOffset, int yOffset){
    bitmap[yOffset+1][xOffset] = 1;
    bitmap[yOffset+3][xOffset] = 1;
}

void drawClock() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    uint8_t sec   = timeinfo->tm_sec;
    uint8_t min   = timeinfo->tm_min;
    uint8_t hour  = timeinfo->tm_hour;
    uint8_t day   = timeinfo->tm_mday;
    uint8_t month = timeinfo->tm_mon + 1;

    if (sec == lastSecond) {
        return;
    }
    lastSecond = sec;

    memset(bitmap, 0, sizeof(bitmap));

    drawDigit(day / 10, 9, 1); 
    drawDigit(day % 10, 13, 1);

    drawDigit(hour / 10, 1, 9);
    drawDigit(hour % 10, 5, 9);

    drawDigit(min / 10, 9, 9);
    drawDigit(min % 10, 13, 9);

    drawDigit(sec / 10, 17, 9);
    drawDigit(sec % 10, 21, 9);

    drawDigit(month / 10, 9, 17);
    drawDigit(month % 10, 13, 17);

    croix.setSide(CroixPharma::BOTH);
    croix.writeBitmap(bitmap);
}

int main(){
    if (wiringPiSetupGpio() < 0) {
        return 1;
    }

    croix.begin();

    while (true) {
        drawClock();
        delay(20);
    }

    return 0;
}
