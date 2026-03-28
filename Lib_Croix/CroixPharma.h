#pragma once
#include <cstdint>
#include <wiringPi.h>

#define PANELS 5
#define SIZE 24

class CroixPharma {
public:
    enum Side { LEFT, RIGHT, BOTH };

    CroixPharma();
    void begin();
    void setSide(Side s);
    void clear();
    void writeBitmap(uint8_t data[SIZE][SIZE]);

private:
    struct SidePins { uint8_t OE,LATCH,CLOCK,DATA; bool rotated; };
    SidePins SIDE_LEFT  = {24,25,8,7,false};
    SidePins SIDE_RIGHT = {14,15,18,23,true}; 
    SidePins activeSides[2];
    Side selectedSide;

    uint8_t bitmap[SIZE][SIZE];
    uint8_t matrix[PANELS][8][8];

    void clearHardware();
    void sendBit(bool b, SidePins side);
    void mapBitmapToPanels(SidePins side);
    void renderCrossSide(SidePins side);
};
