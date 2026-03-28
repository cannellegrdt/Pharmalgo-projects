#include <cstdint>
#include <cstring>
#include "CroixPharma.h"

CroixPharma croix;
#define SIZE 24
uint8_t bitmap[SIZE][SIZE];

struct FontChar {
    char letter;
    const uint8_t* data;
    uint8_t width;
};

#define R3(a,b,c) ((a<<2)|(b<<1)|c)
#define R5(a,b,c,d,e) ((a<<4)|(b<<3)|(c<<2)|(d<<1)|e)

#define FONT3(r0,r1,r2,r3,r4) { r0,r1,r2,r3,r4 }
#define FONT5(r0,r1,r2,r3,r4) { r0,r1,r2,r3,r4 }


const uint8_t H_data[] = FONT3(R3(1,0,1),R3(1,0,1),R3(1,1,1),R3(1,0,1),R3(1,0,1));
const uint8_t A_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,1,1),R3(1,0,1),R3(1,0,1));
const uint8_t C_data[] = FONT3(R3(1,1,1),R3(1,0,0),R3(1,0,0),R3(1,0,0),R3(1,1,1));
const uint8_t K_data[] = FONT3(R3(1,0,1),R3(1,0,1),R3(1,1,0),R3(1,0,1),R3(1,0,1));
const uint8_t O_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,0,1),R3(1,0,1),R3(1,1,1));
const uint8_t T_data[] = FONT3(R3(1,1,1),R3(0,1,0),R3(0,1,0),R3(0,1,0),R3(0,1,0));
const uint8_t P_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,1,1),R3(1,0,0),R3(1,0,0));
const uint8_t R_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,1,0),R3(1,0,1),R3(1,0,1));
const uint8_t L_data[] = FONT3(R3(1,0,0),R3(1,0,0),R3(1,0,0),R3(1,0,0),R3(1,1,1));
const uint8_t G_data[] = FONT3(R3(1,1,1),R3(1,0,0),R3(1,0,1),R3(1,0,1),R3(1,1,1));
const uint8_t M_data[] = FONT5(R5(1,0,0,0,1),R5(1,1,0,1,1),R5(1,0,1,0,1),R5(1,0,0,0,1),R5(1,0,0,0,1));
const uint8_t N_data[] = FONT5(R5(1,0,0,0,1),R5(1,1,0,0,1),R5(1,0,1,0,1),R5(1,0,0,1,1),R5(1,0,0,0,1));
const uint8_t DASH_data[] = FONT3(R3(0,0,0),R3(0,0,0),R3(1,1,1),R3(0,0,0),R3(0,0,0));
const uint8_t SLASH_data[] = FONT3(R3(0,0,1),R3(0,1,0),R3(0,1,0),R3(1,0,0),R3(1,0,0));
const uint8_t SPACE_data[] = FONT3(R3(0,0,0),R3(0,0,0),R3(0,0,0),R3(0,0,0),R3(0,0,0));

const uint8_t ZERO_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,0,1),R3(1,0,1),R3(1,1,1));
const uint8_t ONE_data[] = FONT3(R3(0,1,0),R3(1,1,0),R3(0,1,0),R3(0,1,0),R3(1,1,1));
const uint8_t TWO_data[] = FONT3(R3(1,1,1),R3(0,0,1),R3(1,1,1),R3(1,0,0),R3(1,1,1));
const uint8_t THREE_data[] = FONT3(R3(1,1,1),R3(0,0,1),R3(0,1,1),R3(0,0,1),R3(1,1,1));
const uint8_t FOUR_data[] = FONT3(R3(1,0,1),R3(1,0,1),R3(1,1,1),R3(0,0,1),R3(0,0,1));
const uint8_t FIVE_data[] = FONT3(R3(1,1,1),R3(1,0,0),R3(1,1,1),R3(0,0,1),R3(1,1,1));
const uint8_t SIX_data[] = FONT3(R3(1,1,1),R3(1,0,0),R3(1,1,1),R3(1,0,1),R3(1,1,1));
const uint8_t SEVEN_data[] = FONT3(R3(1,1,1),R3(0,0,1),R3(0,1,0),R3(1,0,0),R3(1,0,0));
const uint8_t EIGHT_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,1,1),R3(1,0,1),R3(1,1,1));
const uint8_t NINE_data[] = FONT3(R3(1,1,1),R3(1,0,1),R3(1,1,1),R3(0,0,1),R3(1,1,1));

FontChar font[] = {
    {'H', H_data, 3},
    {'A', A_data, 3},
    {'C', C_data, 3},
    {'K', K_data, 3},
    {'O', O_data, 3},
    {'T', T_data, 3},
    {'P', P_data, 3},
    {'R', R_data, 3},
    {'L', L_data, 3},
    {'G', G_data, 3},
    {'M', M_data, 5},
    {'N', N_data, 5},
    {'0', ZERO_data, 3},
    {'1', ONE_data, 3},
    {'2', TWO_data, 3},
    {'3', THREE_data, 3},
    {'4', FOUR_data, 3},
    {'5', FIVE_data, 3},
    {'6', SIX_data, 3},
    {'7', SEVEN_data, 3},
    {'8', EIGHT_data, 3},
    {'9', NINE_data, 3},
    {' ', SPACE_data, 3},
    {'-', DASH_data, 3},
    {'/', SLASH_data, 3},
};

const FontChar* getChar(char c){
    for (int i=0; i < sizeof(font)/sizeof(FontChar); i++)
        if (font[i].letter == c) return &font[i];
    return nullptr;
}

bool getPixel(const uint8_t* data, int x, int y, int width){
    return (data[y] >> (width-1-x)) & 1;
}

void drawCharAt(const FontChar* fc, int xOffset, int yOffset){
    if (!fc) return;
    for (int y=0; y<5; y++){
        for (int x=0; x<fc->width; x++){
            if (getPixel(fc->data, x, y, fc->width)){
                int xx = xOffset + x;
                int yy = yOffset + y;
                if (xx>=0 && xx<SIZE && yy>=0 && yy<SIZE)
                    bitmap[yy][xx]=1;
            }
        }
    }
}

int getTextWidth(const char* text){
    int w=0;
    for (int i=0; text[i]; i++){
        const FontChar* fc = getChar(text[i]);
        if (fc) w += fc->width + 1;
    }
    return w;
}

const char* message = "HACKATHON PHARMALGO 27/03-29/03";

int scrollX;
int textWidth;

void scrollMessage(){
    memset(bitmap, 0, sizeof(bitmap));

    int cursor = scrollX;

    for (int i=0; message[i]; i++){
        const FontChar* fc = getChar(message[i]);
        if (!fc) continue;
        drawCharAt(fc, cursor, 9);
        cursor += fc->width + 1;
    }

    croix.setSide(CroixPharma::BOTH);
    croix.writeBitmap(bitmap);

    scrollX--;
    if (scrollX < -textWidth) scrollX = SIZE;
}

int main() {
    if (wiringPiSetupGpio() < 0) {
        return 1;
    }

    croix.begin();
    textWidth = getTextWidth(message);
    scrollX = SIZE;

    while (true) {
        scrollMessage();
        delay(200);
    }

    return 0;
}
