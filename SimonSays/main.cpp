#include "../Lib_Croix/CroixPharma.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <random>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#ifndef SIMULATOR
#include <wiringPi.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#endif
#define KEY_UP    1000
#define KEY_DOWN  1001
#define KEY_LEFT  1002
#define KEY_RIGHT 1003

enum Direction { DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 };
enum Panel     { PANEL_LEFT = 0, PANEL_RIGHT = 1, PANEL_TOP = 2, PANEL_CENTER = 3, PANEL_BOTTOM = 4 };

static Panel direction_to_panel(uint8_t dir) {
    switch (dir) {
    case DIR_UP:    return PANEL_TOP;
    case DIR_DOWN:  return PANEL_BOTTOM;
    case DIR_LEFT:  return PANEL_LEFT;
    case DIR_RIGHT: return PANEL_RIGHT;
    default:        return PANEL_CENTER;
    }
}

static const int MAX_ROUNDS = 10;
uint8_t game[MAX_ROUNDS] = {0};
uint8_t bitmap[SIZE][SIZE] = {{0}};
int current_round = 0;      // 0-based index
bool player_turn = false;
bool loose = false;
bool win = false;
bool extra_life = true;

#ifndef SIMULATOR
CroixPharma croix;
#endif

static struct termios original_termios;
static bool termios_saved = false;

// Big 5x5 digit patterns (holes)
static const uint8_t digit5x5[10][5] = {
    {0b01110, 0b10001, 0b10011, 0b10101, 0b01110}, // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b01110}, // 1
    {0b01110, 0b10001, 0b00010, 0b00100, 0b11111}, // 2
    {0b11110, 0b00001, 0b00110, 0b00001, 0b11110}, // 3
    {0b00010, 0b00110, 0b01010, 0b11111, 0b00010}, // 4
    {0b11111, 0b10000, 0b11110, 0b00001, 0b11110}, // 5
    {0b01110, 0b10000, 0b11110, 0b10001, 0b01110}, // 6
    {0b11111, 0b00001, 0b00010, 0b00100, 0b00100}, // 7
    {0b01110, 0b10001, 0b01110, 0b10001, 0b01110}, // 8
    {0b01110, 0b10001, 0b01111, 0b00001, 0b01110}  // 9
};

// 8x8 heart frame (thicker, bigger)
static const uint8_t heart8x8[8][8] = {
    {0,1,1,0,0,1,1,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0}
};

#ifdef SIMULATOR
static int sim_sock = -1;
static struct sockaddr_in sim_addr;
#endif

void restore_input() {
    if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    }
}

void handle_signal(int sig) {
    restore_input();
    signal(sig, SIG_DFL);
    raise(sig);
}

void setup_signal_handlers() {
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP,  handle_signal);
    signal(SIGQUIT, handle_signal);
}

static const char *get_sound_player() {
#ifdef __APPLE__
    return "afplay";
#else
    return "aplay";
#endif
}

static void play_sound_async(const char *file_path) {
#ifndef SIMULATOR
    std::string cmd = std::string(get_sound_player()) + " \"" + file_path + "\" >/dev/null 2>&1 &";
    system(cmd.c_str());
#else
    (void)file_path;
#endif
}

void set_pixel(int x, int y, uint8_t val) {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) {
        return;
    }
    bool in_top = (x >= 8 && x < 16 && y >= 0  && y < 8);
    bool in_mid = (x >= 0 && x < 24 && y >= 8  && y < 16);
    bool in_bot = (x >= 8 && x < 16 && y >= 16 && y < 24);
    if (!in_top && !in_mid && !in_bot) {
        return;
    }
    bitmap[y][x] = val ? 7 : 0;
}

void clear_frame() {
    memset(bitmap, 0, sizeof(bitmap));
}

void fill_frame() {
    memset(bitmap, 7, sizeof(bitmap));
}

#ifdef SIMULATOR
void init_sim_socket() {
    sim_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sim_sock < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sim_addr, 0, sizeof(sim_addr));
    sim_addr.sin_family = AF_INET;
    sim_addr.sin_port = htons(1337);
    sim_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}
void send_frame_sim() {
    std::string json;
    json.reserve(24 * 24 * 4);
    json.push_back('[');
    for (int r = 0; r < SIZE; r++) {
        if (r > 0) json.push_back(',');
        json.push_back('[');
        for (int c = 0; c < SIZE; c++) {
            if (c > 0) json.push_back(',');
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", bitmap[r][c]);
            json.append(buf);
        }
        json.push_back(']');
    }
    json.push_back(']');
    sendto(sim_sock, json.data(), json.size(), 0,
           (struct sockaddr *)&sim_addr, sizeof(sim_addr));
}
#endif
void send_frame() {
#ifdef SIMULATOR
    send_frame_sim();
#else
    croix.writeBitmap(bitmap);
#endif
}

void invert_frame() {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            bitmap[y][x] = (bitmap[y][x] ? 0 : 7);
        }
    }
}

void setup_input() {
    struct termios t;
    if (tcgetattr(STDIN_FILENO, &t) != 0) {
        return;
    }
    original_termios = t;
    termios_saved = true;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void flush_input() {
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        // discard
    }
}

int get_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    if (c == '\x1b') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return -1;
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return -1;
        }
        if (seq[0] == '[') {
            if (seq[1] == 'A') return KEY_UP;
            if (seq[1] == 'B') return KEY_DOWN;
            if (seq[1] == 'C') return KEY_RIGHT;
            if (seq[1] == 'D') return KEY_LEFT;
        }
        return -1;
    }
    return (unsigned char)c;
}

bool check_correct(int round, int key_code) {
    bool ok = false;
    if (key_code == KEY_UP    && game[round] == DIR_UP)    ok = true;
    if (key_code == KEY_DOWN  && game[round] == DIR_DOWN)  ok = true;
    if (key_code == KEY_LEFT  && game[round] == DIR_LEFT)  ok = true;
    if (key_code == KEY_RIGHT && game[round] == DIR_RIGHT) ok = true;
    return ok;
}

// Return the digit to show for "current round" (1..10 clamped to 0..9)
int current_round_digit() {
    int d = current_round + 1; // round 0 -> "1" key, etc.
    if (d < 0) d = 0;
    if (d > 9) d = 9;
    return d;
}

// Draw big center: full 8x8 ON, then 8x8 heart + 5x5 digit as OFF
void draw_center() {
    int digit = current_round_digit();
    // center 8x8 ON
    for (int y = 8; y < 16; y++) {
        for (int x = 8; x < 16; x++) {
            set_pixel(x, y, 7);
        }
    }
    // heart ring
    if (extra_life) {
        for (int hy = 0; hy < 8; hy++) {
            for (int hx = 0; hx < 8; hx++) {
                if (heart8x8[hy][hx]) {
                    int px = 8 + hx;
                    int py = 8 + hy;
                    set_pixel(px, py, 0);
                }
            }
        }
    }
    // digit 5x5 centered
    int digit_offset_x = 8 + (8 - 5) / 2; // 9
    int digit_offset_y = 8 + (8 - 5) / 2; // 9
    for (int dy = 0; dy < 5; dy++) {
        uint8_t row = digit5x5[digit][dy];
        for (int dx = 0; dx < 5; dx++) {
            bool on = (row >> (4 - dx)) & 0x1;
            if (on) {
                int px = digit_offset_x + dx;
                int py = digit_offset_y + dy;
                set_pixel(px, py, 0);
            }
        }
    }
}

void update_center() {
    // clear only center
    for (int y = 8; y < 16; y++) {
        for (int x = 8; x < 16; x++) {
            set_pixel(x, y, 0);
        }
    }
    draw_center();
}

void flash_full_grid(int times, useconds_t on_us, useconds_t off_us) {
    for (int i = 0; i < times; i++) {
        fill_frame();
        send_frame();
        usleep(on_us);
        clear_frame();
        send_frame();
        usleep(off_us);
    }
    update_center();
    send_frame();
}

void strobe_invert(int times, useconds_t delay_us) {
    for (int i = 0; i < times; i++) {
        invert_frame();
        send_frame();
        usleep(delay_us);
    }
}

void show_panel_additive(Panel panel) {
    const char *sound_file = NULL;
    if (panel == PANEL_LEFT) {
        sound_file = "left.wav";
        for (int y = 8; y < 16; y++)
            for (int x = 0; x < 8; x++)
                set_pixel(x, y, 7);
    } else if (panel == PANEL_RIGHT) {
        sound_file = "right.wav";
        for (int y = 8; y < 16; y++)
            for (int x = 16; x < 24; x++)
                set_pixel(x, y, 7);
    } else if (panel == PANEL_TOP) {
        sound_file = "top.wav";
        for (int y = 0; y < 8; y++)
            for (int x = 8; x < 16; x++)
                set_pixel(x, y, 7);
    } else if (panel == PANEL_BOTTOM) {
        sound_file = "bottom.wav";
        for (int y = 16; y < 24; y++)
            for (int x = 8; x < 16; x++)
                set_pixel(x, y, 7);
    }
    if (sound_file != NULL) {
        play_sound_async(sound_file);
    }
    send_frame();
}

void clear_panel_only(Panel panel) {
    if (panel == PANEL_LEFT) {
        for (int y = 8; y < 16; y++)
            for (int x = 0; x < 8; x++)
                set_pixel(x, y, 0);
    } else if (panel == PANEL_RIGHT) {
        for (int y = 8; y < 16; y++)
            for (int x = 16; x < 24; x++)
                set_pixel(x, y, 0);
    } else if (panel == PANEL_TOP) {
        for (int y = 0; y < 8; y++)
            for (int x = 8; x < 16; x++)
                set_pixel(x, y, 0);
    } else if (panel == PANEL_BOTTOM) {
        for (int y = 16; y < 24; y++)
            for (int x = 8; x < 16; x++)
                set_pixel(x, y, 0);
    }
}

void show_round(int round) {
    // center already shows current_round+1 before machine turn
    for (int i = 0; i <= round; i++) {
        Panel panel = direction_to_panel(game[i]);
        show_panel_additive(panel);
        usleep(500000);
        clear_panel_only(panel);
        update_center();
        send_frame();
        usleep(200000);
    }
    usleep(600000);
}

void show_current_round(int round) {
    Panel panel = direction_to_panel(game[round]);
    show_panel_additive(panel);
    usleep(500000);
    clear_panel_only(panel);
    update_center();
    send_frame();
    usleep(200000);
}

void game_loose_final() {
    play_sound_async("loose.wav");
    flash_full_grid(3, 150000, 150000);
    fill_frame();
    send_frame();
    usleep(300000);
    // show digit for the round you died on
    draw_center();
    send_frame();
    usleep(700000);
    strobe_invert(10, 100000);
}

void game_win() {
    fill_frame();
    send_frame();
}

int main(void) {
    std::mt19937 rng(static_cast<uint32_t>(time(NULL)));
    std::uniform_int_distribution<int> dist(0, 3);
    for (int i = 0; i < MAX_ROUNDS; i++) {
        game[i] = static_cast<uint8_t>(dist(rng));
    }
#ifndef SIMULATOR
    if (wiringPiSetupGpio() < 0) {
        return 1;
    }
    croix.begin();
    croix.setSide(CroixPharma::BOTH);
#else
    init_sim_socket();
#endif
    setup_input();
    atexit(restore_input);
    setup_signal_handlers();
    clear_frame();
    send_frame();
    update_center();
    send_frame();
    usleep(800000);
    while (true) {
        if (!player_turn && !loose && !win) {
            show_round(current_round);
            flush_input();
            player_turn = true;
        } else if (player_turn && !loose && !win) {
            bool local_loose = false;
            for (int i = 0; i <= current_round; i++) {
                bool correct = false;
                while (!correct) {
                    int key = get_key();
                    if (key == KEY_UP || key == KEY_DOWN ||
                        key == KEY_LEFT || key == KEY_RIGHT) {
                        if (check_correct(i, key)) {
                            show_current_round(i);
                            correct = true;
                        } else {
                            local_loose = true;
                            correct = true;
                        }
                    } else {
                        usleep(10000);
                    }
                }
                if (local_loose) break;
            }
            if (local_loose) {
                if (extra_life) {
                    flash_full_grid(3, 150000, 150000);
                    extra_life = false;
                    update_center();
                    send_frame();
                    player_turn = false;
                } else {
                    loose = true;
                }
            } else {
                // finished this round successfully, go to next
                current_round++;
                if (current_round >= MAX_ROUNDS) {
                    win = true;
                }
                update_center();   // center digit updates to new round
                send_frame();
                player_turn = false;
            }
        } else if (loose) {
            game_loose_final();
            break;
        } else if (win) {
            game_win();
            break;
        }
        usleep(10000);
    }
    return 0;
}
