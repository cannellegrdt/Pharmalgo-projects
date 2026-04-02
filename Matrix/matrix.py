##
## EPITECH PROJECT, 2025
## Pharmalgo
## File description:
## matrix.py
##

#!/usr/bin/env python3
"""
Infinite Matrix rain effect for the pharmacy cross LED display.

Falling streams of light drip down each column of the cross shape,
with a bright head and a fading green-style trail (brightness 7 → 1).

Cross layout (24x24 grid, 5 panels of 8x8):
    cols 8-15 : rows 0-7   (TOP)
    cols 0-7  : rows 8-15  (LEFT)
    cols 8-15 : rows 8-15  (CENTER)
    cols 16-23: rows 8-15  (RIGHT)
    cols 8-15 : rows 16-23 (BOTTOM)
"""

import json
import random
import socket
import time

UDP_HOST = "127.0.0.1"
UDP_PORT = 1337

FPS = 20

COLUMN_RANGES = {
    **{c: (8, 16) for c in range(0, 8)},
    **{c: (0, 24) for c in range(8, 16)},
    **{c: (8, 16) for c in range(16, 24)},
}

TRAIL_LEN = 5


class Stream:
    def __init__(self, col: int):
        self.col = col
        row_min, row_max = COLUMN_RANGES[col]
        self.row_min = row_min
        self.row_max = row_max
        self._reset(start_above=True)

    def _reset(self, start_above: bool = False):
        if start_above:
            self.head = self.row_min - random.randint(1, self.row_max - self.row_min)
        else:
            self.head = self.row_min - 1
        self.speed = random.choice([1, 1, 1, 2])

    def step(self):
        self.head += self.speed
        if self.head - TRAIL_LEN >= self.row_max:
            self._reset()

    def draw(self, frame: list):
        for i in range(TRAIL_LEN + 1):
            row = self.head - i
            if self.row_min <= row < self.row_max:
                if i == 0:
                    brightness = 7
                else:
                    brightness = max(1, 7 - i * (7 // TRAIL_LEN) - 1)
                frame[row][self.col] = max(frame[row][self.col], brightness)


def blank_frame() -> list:
    return [[0] * 24 for _ in range(24)]


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    streams = [Stream(col) for col in range(24)]

    print("╔══════════════════════════════════════╗")
    print("║     Matrix Rain - Pharmacy Cross     ║")
    print("╚══════════════════════════════════════╝")
    print()
    print("Ctrl+C to stop.")

    interval = 1.0 / FPS
    try:
        while True:
            frame = blank_frame()
            for stream in streams:
                stream.step()
                stream.draw(frame)

            payload = json.dumps(frame).encode("utf-8")
            sock.sendto(payload, (UDP_HOST, UDP_PORT))
            time.sleep(interval)

    except KeyboardInterrupt:
        print("\nStopped - clearing display.")
        blank = json.dumps(blank_frame()).encode()
        sock.sendto(blank, (UDP_HOST, UDP_PORT))


if __name__ == "__main__":
    main()
