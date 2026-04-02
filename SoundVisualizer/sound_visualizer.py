##
## EPITECH PROJECT, 2025
## Pharmalgo
## File description:
## sound_visualizer.py
##

#!/usr/bin/env python3
"""
Real-time sound visualizer for the pharmacy cross LED display.

Each panel covers a frequency range, split into 8 sub-bands.
Each sub-band drives one independent bar growing outward from center:

  TOP    (4k-20kHz) : 8 vertical bars, one per column, growing upward
  RIGHT  (800-4kHz) : 8 horizontal bars, one per row, growing rightward
  BOTTOM (150-800Hz): 8 vertical bars, one per column, growing downward
  LEFT   (20-150Hz) : 8 horizontal bars, one per row, growing leftward
  CENTER            : stays dark

Requirements:
    pip install sounddevice numpy
"""

import math
import socket
import json
import sys
import numpy as np
import sounddevice as sd

UDP_HOST = "127.0.0.1"
UDP_PORT = 1337

SAMPLE_RATE = 44100
CHUNK = 2048

PANEL_OFFSETS = {
    "top": (0, 8),
    "left": (8, 0),
    "center": (8, 8),
    "right": (8, 16),
    "bottom": (16, 8),
}

PANEL_FREQ = {
    "top": (4000, 20000), # Treble
    "right": (800, 4000), # High-mid
    "bottom": (150, 800), # Mid
    "left": (20, 150), # Bass
    # center intentionally absent - stays dark
}

PANEL_DIR = {
    "top": "up",
    "bottom": "down",
    "left": "left",
    "right": "right",
}

SMOOTHING = 0.65
PEAK_DECAY = 0.993
N_BARS = 8

def log_split(fmin: float, fmax: float, n: int) -> list:
    """Split [fmin, fmax] into n sub-bands with logarithmic spacing."""
    log_min = math.log(max(fmin, 1))
    log_max = math.log(fmax)
    edges = [math.exp(log_min + i * (log_max - log_min) / n) for i in range(n + 1)]
    return [(edges[i], edges[i + 1]) for i in range(n)]


def make_panel(levels: list, direction: str) -> list:
    """
    Build an 8x8 panel with N_BARS independent bars.
    Each bar grows outward from the cross center.

    For vertical panels (up/down): bar_idx = column, bar grows in row direction.
    For horizontal panels (left/right): bar_idx = row, bar grows in col direction.

    levels: list of N_BARS floats in [0.0, 1.0]
    """
    panel = [[0] * 8 for _ in range(8)]

    for bar_idx, level in enumerate(levels):
        lit = int(round(level * N_BARS))
        if lit == 0:
            continue

        for step in range(lit):
            brightness = max(2, 7 - step)

            if direction == "up":
                panel[7 - step][bar_idx] = brightness

            elif direction == "down":
                panel[step][bar_idx] = brightness

            elif direction == "left":
                panel[bar_idx][7 - step] = brightness

            elif direction == "right":
                panel[bar_idx][step] = brightness

    return panel


def band_energy(magnitudes: np.ndarray, freq_min: float, freq_max: float) -> float:
    """RMS energy for a frequency band."""
    res = SAMPLE_RATE / CHUNK
    b_min = max(0, int(freq_min / res))
    b_max = min(len(magnitudes), int(freq_max / res) + 1)
    if b_min >= b_max:
        return 0.0
    return float(np.sqrt(np.mean(magnitudes[b_min:b_max] ** 2)))


def panels_to_frame(panels: dict) -> list:
    frame = [[0] * 24 for _ in range(24)]
    for name, (row_off, col_off) in PANEL_OFFSETS.items():
        p = panels.get(name)
        for row in range(8):
            for col in range(8):
                frame[row_off + row][col_off + col] = p[row][col] if p else 0
    return frame


_smoothed: dict = {}
_peak: dict = {}
_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

_sub_bands: dict = {name: log_split(fmin, fmax, N_BARS)
                    for name, (fmin, fmax) in PANEL_FREQ.items()}


def audio_callback(indata, frames, time_info, status):
    audio = indata[:, 0]
    windowed = audio * np.hanning(len(audio))
    fft = np.fft.rfft(windowed)
    magnitudes = np.abs(fft) / CHUNK

    panels = {}
    for panel_name, sub_bands in _sub_bands.items():
        direction = PANEL_DIR[panel_name]
        levels = [] # B

        for bar_idx, (fmin, fmax) in enumerate(sub_bands):
            key = f"{panel_name}:{bar_idx}"
            raw = band_energy(magnitudes, fmin, fmax)

            _peak[key] = max(_peak.get(key, 1e-6) * PEAK_DECAY, raw, 1e-6)
            normalized = min(raw / _peak[key], 1.0)
            _smoothed[key] = SMOOTHING * _smoothed.get(key, 0.0) + (1.0 - SMOOTHING) * normalized

            levels.append(_smoothed[key])

        panels[panel_name] = make_panel(levels, direction)

    frame   = panels_to_frame(panels)
    payload = json.dumps(frame).encode("utf-8")
    _sock.sendto(payload, (UDP_HOST, UDP_PORT))


def main():
    print("╔══════════════════════════════════════╗")
    print("║   Sound Visualizer — Pharmacy Cross  ║")
    print("╚══════════════════════════════════════╝")
    print()
    print("Panel → frequency range → 8 independent bars:")
    for name, (fmin, fmax) in PANEL_FREQ.items():
        print(f"  {name:8s} ({PANEL_DIR[name]:5s}) → {fmin:5d} – {fmax:6d} Hz")
    print()
    print("Ctrl+C to stop.")
    print()

    try:
        with sd.InputStream(
            samplerate=SAMPLE_RATE,
            channels=1,
            dtype="float32",
            blocksize=CHUNK,
            callback=audio_callback,
        ):
            while True:
                sd.sleep(100)
    except KeyboardInterrupt:
        print("\nStopped — clearing display.")
        blank = json.dumps([[0] * 24 for _ in range(24)]).encode()
        _sock.sendto(blank, (UDP_HOST, UDP_PORT))
    except sd.PortAudioError as e:
        print(f"Audio error: {e}", file=sys.stderr)
        print("Check that a microphone is connected and accessible.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
