#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../Lib_Croix/CroixPharma.h"

namespace {
std::atomic<bool> g_running{true};

constexpr int kGridSize = 24;
constexpr int kPanelSize = 8;
constexpr int kFrameRate = 10;  // Smooth animation.
constexpr int kFrameTimeMs = 1000 / kFrameRate;

using Grid = std::array<std::array<uint8_t, kGridSize>, kGridSize>;

bool is_drawable(int y, int x)
{
    const int panel_y = y / kPanelSize;
    const int panel_x = x / kPanelSize;
    return (panel_y == 0 && panel_x == 1) || (panel_y == 1 && panel_x == 0)
        || (panel_y == 1 && panel_x == 1) || (panel_y == 1 && panel_x == 2)
        || (panel_y == 2 && panel_x == 1);
}

int count_neighbors(const Grid &grid, int y, int x)
{
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dy == 0 && dx == 0) {
                continue;
            }

            const int ny = y + dy;
            const int nx = x + dx;

            if (ny < 0 || ny >= kGridSize || nx < 0 || nx >= kGridSize) {
                continue;
            }
            if (!is_drawable(ny, nx)) {
                continue;
            }
            if (grid[ny][nx] > 0) {
                ++count;
            }
        }
    }
    return count;
}

Grid compute_next_generation(const Grid &current)
{
    Grid next{};

    for (int y = 0; y < kGridSize; ++y) {
        for (int x = 0; x < kGridSize; ++x) {
            if (!is_drawable(y, x)) {
                next[y][x] = 0;
                continue;
            }

            const int neighbors = count_neighbors(current, y, x);
            const uint8_t cell = current[y][x];

            if (cell > 0) {
                next[y][x] = (neighbors == 2 || neighbors == 3) ? 7 : 0;
            } else {
                next[y][x] = (neighbors == 3) ? 7 : 0;
            }
        }
    }

    return next;
}

Grid initialize_grid()
{
    Grid grid{};
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 99);

    for (int y = 0; y < kGridSize; ++y) {
        for (int x = 0; x < kGridSize; ++x) {
            if (is_drawable(y, x) && dist(rng) < 40) {
                grid[y][x] = 7;
            }
        }
    }

    // Seed a glider-like pattern near center to kickstart movement.
    if (is_drawable(10, 12)) grid[10][12] = 7;
    if (is_drawable(11, 13)) grid[11][13] = 7;
    if (is_drawable(12, 11)) grid[12][11] = 7;
    if (is_drawable(12, 12)) grid[12][12] = 7;
    if (is_drawable(12, 13)) grid[12][13] = 7;

    return grid;
}

void send_grid_to_simulator(const Grid &grid)
{
    static int sock = -1;
    static sockaddr_in addr{};
    static bool initialized = false;

    if (!initialized) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            return;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(1337);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        initialized = true;
    }

    char buffer[8192];
    int pos = 0;

    pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), "[");
    for (int y = 0; y < kGridSize; ++y) {
        if (y > 0) {
            pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), ",");
        }
        pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), "[");

        for (int x = 0; x < kGridSize; ++x) {
            if (x > 0) {
                pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), ",");
            }
            pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), "%d", grid[y][x]);
        }

        pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), "]");
    }
    pos += snprintf(buffer + pos, sizeof(buffer) - static_cast<size_t>(pos), "]");

    if (pos > 0) {
        sendto(sock, buffer, static_cast<size_t>(pos), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    }
}

void write_grid_to_hardware(CroixPharma *croix, const Grid &grid)
{
    if (croix == nullptr) {
        return;
    }

    uint8_t bitmap[SIZE][SIZE] = {};
    for (int y = 0; y < kGridSize; ++y) {
        for (int x = 0; x < kGridSize; ++x) {
            bitmap[y][x] = is_drawable(y, x) ? grid[y][x] : 0;
        }
    }
    croix->writeBitmap(bitmap);
}

void show_startup_pattern(CroixPharma *croix)
{
    if (croix == nullptr) {
        return;
    }

    uint8_t bitmap[SIZE][SIZE] = {};
    for (int y = 0; y < kGridSize; ++y) {
        for (int x = 0; x < kGridSize; ++x) {
            if (is_drawable(y, x)) {
                bitmap[y][x] = HIGH;
            }
        }
    }

    croix->writeBitmap(bitmap);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void signal_handler(int)
{
    g_running = false;
}

}  // namespace

int main()
{
    std::signal(SIGINT, signal_handler);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    CroixPharma *croix = nullptr;
    bool has_hardware = false;

    if (wiringPiSetupGpio() >= 0) {
        croix = new CroixPharma();
        croix->begin();
        croix->setSide(CroixPharma::BOTH);
        has_hardware = true;
        printf("Hardware mode enabled (Lib_Croix).\n");
    } else {
        fprintf(stderr, "ERROR: wiringPiSetupGpio failed, cross hardware not initialized.\n");
        fprintf(stderr, "Check GPIO access (sudo) and wiring.\n");
        return 1;
    }

    Grid grid = initialize_grid();

    if (has_hardware) {
        show_startup_pattern(croix);
    }

    printf("Game of Life started. Press Ctrl+C to stop.\n");
    printf("Target speed: %d generations/sec\n", kFrameRate);

    int generation = 0;
    const auto start_time = std::chrono::steady_clock::now();

    while (g_running) {
        send_grid_to_simulator(grid);
        write_grid_to_hardware(croix, grid);

        grid = compute_next_generation(grid);
        ++generation;

        if (generation % kFrameRate == 0) {
            const auto now = std::chrono::steady_clock::now();
            const auto elapsed_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            const double real_rate = elapsed_ms > 0 ? (generation * 1000.0) / elapsed_ms : 0.0;
            printf("\rGeneration %5d (%.1f gen/s)", generation, real_rate);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(kFrameTimeMs));
    }

    printf("\nStopped after %d generations.\n", generation);

    if (croix != nullptr) {
        croix->clear();
        delete croix;
    }

    return 0;
}
