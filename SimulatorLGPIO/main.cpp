#include "../Simulator/sim.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TOTAL_NB_LEDS 320

struct Panel {
    int start;
    int fr;
    int fc;
};

static const Panel PANELS[5] = {
    { 0, 8, 0 },
    { 64, 8, 16 },
    { 128, 0, 8 },
    { 192, 8, 8 },
    { 256, 16, 8 },
};

static int parseFrame(const char *json, uint8_t frame[24][24]) {
    int count = 0;
    for (const char *p = json; *p && count < 576; p++) {
        if (*p >= '0' && *p <= '9') {
            if (p > json && *(p - 1) >= '0' && *(p - 1) <= '9')
                continue;
            frame[count / 24][count % 24] = (uint8_t)(*p - '0');
            count++;
        }
    }
    return count == 576;
}

static int parseSection(const char *key, const char *json, uint8_t frame[24][24]) {
    const char *p = strstr(json, key);
    if (!p) return 0;
    p = strchr(p, '[');
    if (!p) return 0;

    int count = 0;
    for (; *p && count < 576; p++) {
        if (*p >= '0' && *p <= '9') {
            if (p > json && *(p - 1) >= '0' && *(p - 1) <= '9')
                continue;
            frame[count / 24][count % 24] = (uint8_t)(*p - '0');
            count++;
        }
    }
    return count == 576;
}

static int parseDualFrame(const char *json, uint8_t recto[24][24], uint8_t verso[24][24]) {
    return parseSection("\"recto\"", json, recto) &&
           parseSection("\"verso\"", json, verso);
}

static void frameToPixels(const uint8_t frame[24][24], uint8_t pixels[TOTAL_NB_LEDS]) {
    memset(pixels, 0, TOTAL_NB_LEDS);
    for (const Panel &pan : PANELS) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                pixels[pan.start + row * 8 + (7 - col)] =
                    frame[pan.fr + row][pan.fc + col];
            }
        }
    }
}

static void sendFrame(const uint8_t pixels[TOTAL_NB_LEDS]) {
    sim(SORTIE_3, HIGH);
    for (int i = 0; i < TOTAL_NB_LEDS; i++) {
        sim(SORTIE_4, pixels[i]);
        sim(SORTIE_2, HIGH);
    }
    sim(SORTIE_1, HIGH);
}

static void sendFrameFace(const uint8_t pixels[TOTAL_NB_LEDS], uint8_t face) {
    sim_face(face, SORTIE_3, HIGH);
    for (int i = 0; i < TOTAL_NB_LEDS; i++) {
        sim_face(face, SORTIE_4, pixels[i]);
        sim_face(face, SORTIE_2, HIGH);
    }
    sim_face(face, SORTIE_1, HIGH);
}

int main() {
    sim_init();

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("[challenge_hw] socket");
        return 1;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1337);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[challenge_hw] bind");
        return 1;
    }

    fprintf(stderr, "[challenge_hw] Listening on UDP :1337 — waiting for frames\n");

    static char buf[65536];
    static uint8_t frame[24][24];
    static uint8_t recto_frame[24][24];
    static uint8_t verso_frame[24][24];
    static uint8_t pixels[TOTAL_NB_LEDS];
    static uint8_t recto_pixels[TOTAL_NB_LEDS];
    static uint8_t verso_pixels[TOTAL_NB_LEDS];

    while (1) {
        ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) continue;
        buf[n] = '\0';
        
        if (buf[0] == '{') {
            if (!parseDualFrame(buf, recto_frame, verso_frame)) {
                fprintf(stderr, "[challenge_hw] WARNING: malformed dual frame (%zd bytes)\n", n);
                continue;
            }
            frameToPixels(recto_frame, recto_pixels);
            frameToPixels(verso_frame, verso_pixels);
            sendFrameFace(recto_pixels, 0);
            sendFrameFace(verso_pixels, 1);
        } else {
            if (!parseFrame(buf, frame)) {
                fprintf(stderr, "[challenge_hw] WARNING: malformed frame (%zd bytes)\n", n);
                continue;
            }
            frameToPixels(frame, pixels);
            sendFrame(pixels);
        }
    }
}
