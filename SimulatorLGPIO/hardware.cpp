#include "../Simulator/sim.hpp"
#include <lgpio.h>
#include <cstdio>

// Recto (front face)
static constexpr int R_OE = 24;
static constexpr int R_LATCH = 25;
static constexpr int R_CLK = 8;
static constexpr int R_DATA = 7;

// Verso (back face)
static constexpr int V_OE = 14;
static constexpr int V_LATCH = 15;
static constexpr int V_CLK = 18;
static constexpr int V_DATA = 23;
static constexpr int GPIO_CHIP_RPI5 = 4;
static constexpr int GPIO_CHIP_RPI4 = 0;

static int gh = -1;

static inline void w(int recto, int verso, int level) {
    lgGpioWrite(gh, recto, level);
    lgGpioWrite(gh, verso, level);
}

void sim_init() {
    gh = lgGpiochipOpen(GPIO_CHIP_RPI5);
    if (gh < 0)
        gh = lgGpiochipOpen(GPIO_CHIP_RPI4);
    if (gh < 0) {
        fprintf(stderr, "[hardware] ERROR: cannot open GPIO chip\n");
        return;
    }

    const int pins[] = { R_OE, R_LATCH, R_CLK, R_DATA, V_OE, V_LATCH, V_CLK, V_DATA };
    for (int pin : pins) {
        if (lgGpioClaimOutput(gh, 0, pin, 0) < 0)
            fprintf(stderr, "[hardware] WARNING: could not claim GPIO %d\n", pin);
    }
    fprintf(stderr, "[hardware] GPIO ready (chip %d)\n", gh);
}

void sim(uint8_t pin, uint8_t value) {
    const int level = value ? 1 : 0;

    switch (pin) {
        case SORTIE_3:
            w(R_OE, V_OE, level ? 0 : 1);
            break;

        case SORTIE_4:
            w(R_DATA, V_DATA, level);
            break;

        case SORTIE_2:
            if (value) {
                w(R_CLK, V_CLK, 1);
                lguSleep(1e-6);
                w(R_CLK, V_CLK, 0);
            }
            break;

        case SORTIE_1:
            if (value) {
                w(R_LATCH, V_LATCH, 1);
                lguSleep(1e-6);
                w(R_LATCH, V_LATCH, 0);
            }
            break;
    }
}

void sim_face(uint8_t face, uint8_t pin, uint8_t value) {
    const int level = value ? 1 : 0;
    const int oe = (face == 0) ? R_OE : V_OE;
    const int data = (face == 0) ? R_DATA : V_DATA;
    const int clk = (face == 0) ? R_CLK : V_CLK;
    const int latch = (face == 0) ? R_LATCH : V_LATCH;

    switch (pin) {
        case SORTIE_3:
            lgGpioWrite(gh, oe, level ? 0 : 1);
            break;

        case SORTIE_4:
            lgGpioWrite(gh, data, level);
            break;

        case SORTIE_2:
            if (value) {
                lgGpioWrite(gh, clk, 1);
                lguSleep(1e-6);
                lgGpioWrite(gh, clk, 0);
            }
            break;
            
        case SORTIE_1:
            if (value) {
                lgGpioWrite(gh, latch, 1);
                lguSleep(1e-6);
                lgGpioWrite(gh, latch, 0);
            }
            break;
    }
}
