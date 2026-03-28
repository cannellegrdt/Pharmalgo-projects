#include "CroixPharma.h"
#include <cstring>

CroixPharma::CroixPharma() {
    selectedSide = LEFT;
}

void CroixPharma::begin() {
    pinMode(SIDE_LEFT.LATCH,OUTPUT);
    pinMode(SIDE_LEFT.CLOCK,OUTPUT);
    pinMode(SIDE_LEFT.DATA,OUTPUT);
    pinMode(SIDE_LEFT.OE,OUTPUT); 
    digitalWrite(SIDE_LEFT.OE,LOW);
    
    pinMode(SIDE_RIGHT.LATCH,OUTPUT);
    pinMode(SIDE_RIGHT.CLOCK,OUTPUT);
    pinMode(SIDE_RIGHT.DATA,OUTPUT);
    pinMode(SIDE_RIGHT.OE,OUTPUT); 
    digitalWrite(SIDE_RIGHT.OE,LOW);
    
    setSide(selectedSide);
}

void CroixPharma::setSide(Side s) {
    selectedSide = s;
    if (s==LEFT)
        activeSides[0] = SIDE_LEFT;
    else if (s==RIGHT)
        activeSides[0] = SIDE_RIGHT;
    else {
        activeSides[0] = SIDE_LEFT;
        activeSides[1] = SIDE_RIGHT;
    }
}

void CroixPharma::clearHardware() {
    memset(matrix,0,sizeof(matrix));
}

void CroixPharma::sendBit(bool b, SidePins side) {
    digitalWrite(side.DATA, b);
    
    digitalWrite(side.CLOCK, HIGH);
    delayMicroseconds(1); // Obligatoire pour éviter le flash
    digitalWrite(side.CLOCK, LOW);
}

void CroixPharma::mapBitmapToPanels(SidePins side) {
    clearHardware();
    for (int y=0;y<SIZE;y++){
        for (int x=0;x<SIZE;x++){
            bool pixel = bitmap[y][x];
            int bx=x, by=y;

            if (bx>=8 && bx<16 && by<8)
                matrix[2][by][bx-8] = pixel;
            else if (bx<8 && by>=8 && by<16)
                matrix[4][by-8][bx] = pixel;
            else if (bx>=8 && bx<16 && by>=8 && by<16)
                matrix[1][by-8][bx-8] = pixel;
            else if (bx>=16 && by>=8 && by<16)
            matrix[3][by-8][bx-16] = pixel;
            else if (bx>=8 && by>=16)
                matrix[0][by-16][bx-8] = pixel;
        }
    }
}

void CroixPharma::renderCrossSide(SidePins side) {
    mapBitmapToPanels(side);
    
    for (int p=PANELS-1;p>=0;p--) {
        for (int y=0;y<8;y++) {
            for (int x=7;x>=0;x--) {
                sendBit(matrix[p][y][x], side);
            }
        }
    }

    digitalWrite(side.OE, HIGH); 
    
    digitalWrite(side.LATCH, HIGH); 
    delayMicroseconds(1);
    digitalWrite(side.LATCH, LOW);
    
    digitalWrite(side.OE, LOW);
}

void CroixPharma::clear() {
    memset(bitmap,0,sizeof(bitmap));
    if (selectedSide==BOTH) {
        renderCrossSide(SIDE_LEFT);
        renderCrossSide(SIDE_RIGHT);
    } else {
        renderCrossSide(activeSides[0]);
    }
}

void CroixPharma::writeBitmap(uint8_t data[SIZE][SIZE]) {
    memcpy(bitmap,data,sizeof(bitmap));
    if (selectedSide==BOTH) {
        renderCrossSide(SIDE_LEFT);
        renderCrossSide(SIDE_RIGHT);
    } else {
        renderCrossSide(activeSides[0]);
    }
}
