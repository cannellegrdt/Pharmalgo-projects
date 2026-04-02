# Pharmalgo — Pharmacy Cross LED display system

> Control, simulate, and drive a 5-panel LED pharmacy cross — from a browser pixel editor to a Raspberry Pi 5 hardware target.

---

## About

**Pharmalgo** is an end-to-end system for designing and driving the animated LED display of a pharmacy cross. The display consists of five 8×8 LED panels arranged in a **+** shape (TOP, LEFT, CENTER, RIGHT, BOTTOM), each LED supporting **8 brightness levels** (0 = off, 7 = maximum).

The project spans three layers:

- **Simulator** — a Pygame window that renders the 24×24 LED grid in real-time over UDP, so you can develop and test animations without touching any hardware.
- **Web Editor** — a pixel-art editor inspired by Piskel, running in the browser. Draw frames on the cross, switch between the front (Recto) and back (Verso) faces, and stream changes live to the simulator.
- **Hardware driver** — a C++ binary targeting a **Raspberry Pi 5** that receives the same UDP frames and drives the physical shift-register chain via GPIO (lgpio).

The architecture is unified around a simple **UDP JSON protocol** on port 1337, which means the simulator, the editor backend, the sound visualizer, and the hardware target are all drop-in replacements for each other.

This project was developed as part of an EPITECH Hackathon, with a concrete deployment goal on real pharmacy hardware.

---

## Table of contents

- [About](#about)
- [Architecture](#architecture)
- [Project structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
  - [1 — Simulator](#1--simulator)
  - [2 — Web editor](#2--web-editor)
  - [3 — Hardware (Raspberry Pi 5)](#3--hardware-raspberry-pi-5)
  - [4 — Sound visualizer](#4--sound-visualizer)
  - [5 — Cabling test](#5--cabling-test)
- [UDP protocol](#udp-protocol)
- [Panel layout](#panel-layout)
- [Architecture diagram](#architecture-diagram)
- [Demo / Screenshots](#demo--screenshots)
- [Contributing](#contributing)
- [License](#license)

---

## Architecture

```
┌─────────────────────┐        POST /api/frame
│   Browser editor    │ ──────────────────────────┐
│   (Canvas + JS)     │                           ▼
└─────────────────────┘               ┌───────────────────────┐
                                      │    editor/server.py   │
┌─────────────────────┐               │   (Flask, port 8080)  │
│  sound_visualizer   │               └──────────┬────────────┘
│   (Python + FFT)    │                          │
└──────────┬──────────┘             UDP JSON     │
           │                        port 1337    │
           └──────────────────────┬──────────────┘
                                  ▼
                   ┌──────────────────────────────┐
                   │          UDP :1337           │
                   └────────────┬─────────────────┘
                       ┌────────┴─────────┐
                       ▼                  ▼
            ┌──────────────────┐  ┌────────────────────┐
            │      sim.py      │  │   challenge_hw     │
            │  (Pygame, dev)   │  │  (C++, Raspberry   │
            └──────────────────┘  │     Pi 5 GPIO)     │
                                  └────────────────────┘
```

Both consumers speak the same protocol — swap `sim.py` for `challenge_hw` to go from development to production with zero changes.

---

## Project structure

```
pharmalgo/
├── editor/                    # Web pixel editor
│   ├── server.py              # Flask backend + UDP relay
│   ├── requirements.txt
│   └── static/
│       ├── index.html
│       ├── style.css
│       └── editor.js
│
├── simulator/                 # Pygame simulator + C++ hardware driver
│   ├── sim.py                 # Pygame LED renderer (UDP listener)
│   ├── sim.hpp                # Shared C++ header (pin definitions, sim API)
│   ├── main.cpp               # Simulator entry point (links libsim.a)
│   ├── main_hw.cpp            # Hardware entry point (UDP → GPIO)
│   ├── hardware.cpp           # lgpio GPIO implementation
│   └── Makefile
│
└── scripts/
    ├── cablageTest.cpp        # Panel wiring validation test
    └── sound_visualizer.py   # Real-time audio → LED visualizer
```

---

## Prerequisites

### Simulator & editor (development)
- Python 3.8+
- pip
- Pygame (`pip install pygame`)
- Flask (`pip install flask`)

### Sound visualizer
- `sounddevice` and `numpy` (`pip install sounddevice numpy`)
- A working microphone

### Hardware (Raspberry Pi 5)
- `g++` with C++17 support
- `liblgpio-dev` (`sudo apt install liblgpio-dev`)
- The cross wired to the GPIO pins defined in `hardware.cpp`

---

## Installation

**Clone the repository:**
```bash
git clone https://github.com/your-org/pharmalgo.git
cd pharmalgo
```

**Install Python dependencies:**
```bash
pip install pygame
pip install -r editor/requirements.txt
```

**Build the C++ simulator binary:**
```bash
cd simulator
make
```

**Build the hardware binary (Raspberry Pi 5 only):**
```bash
cd simulator
make hardware
```

---

## Usage

### 1 — Simulator

Starts the Pygame window and listens for UDP frames on port 1337.

```bash
cd simulator
python3 sim.py
```

### 2 — Web editor

In a second terminal, start the Flask backend:

```bash
cd editor
python3 server.py
```

Open **http://localhost:8080** in your browser.

#### Editor interface

```
┌──────────┬─────────────────────────────────────────┬──────────────┐
│  Tools   │          [RECTO]   [VERSO]              │   Sidebar    │
│          │                                         │              │
│  Pencil  │                TOP                      │ Active face  │
│  Eraser  │         LEFT  CENTER  RIGHT             │ Brightness   │
│  Fill    │               BOTTOM                    │ Cursor info  │
│  Pick    │                                         │ Keyboard ref │
└──────────┴─────────────────────────────────────────┴──────────────┘
```

#### Keyboard shortcuts

| Key           | Action                  |
|---------------|-------------------------|
| `P`           | Pencil tool             |
| `E`           | Eraser tool             |
| `F`           | Flood fill              |
| `I`           | Eyedropper (pick)       |
| `0` – `7`     | Set brightness level    |
| `S`           | Send active face        |
| `B`           | Send both faces         |
| `Tab`         | Switch Recto ↔ Verso    |
| `Del`         | Clear active face       |

### 3 — Hardware (Raspberry Pi 5)

Build and run `challenge_hw` on the Pi. It binds UDP port 1337 and drives the GPIO shift-register chain.

```bash
# On the Raspberry Pi 5
cd simulator
make hardware
sudo ./challenge_hw
```

Then point the editor (or sound visualizer) at the Pi's IP — set `UDP_HOST` in `editor/server.py` and `scripts/sound_visualizer.py` accordingly.

### 4 — Sound visualizer

Streams real-time microphone audio to the cross. Each panel covers a different frequency band:

| Panel    | Direction | Frequency range  | Content   |
|----------|-----------|------------------|-----------|
| TOP      | ↑ up      | 4 000 – 20 000 Hz | Treble   |
| RIGHT    | → right   | 800 – 4 000 Hz   | High-mid  |
| BOTTOM   | ↓ down    | 150 – 800 Hz     | Mid       |
| LEFT     | ← left    | 20 – 150 Hz      | Bass      |
| CENTER   | —         | —                | (dark)    |

```bash
pip install sounddevice numpy
python3 scripts/sound_visualizer.py
```

### 5 — Cabling test

The `cablageTest.cpp` program displays a known digit on each panel (1 → TOP, 2 → LEFT, 3 → CENTER, 4 → RIGHT, 5 → BOTTOM) to validate wiring and panel offsets.

```bash
cd simulator
make
./challenge        # runs cablageTest() against the simulator
```

---

## UDP protocol

All producers send **JSON over UDP** to port 1337.

### Single-face frame (legacy)
```json
[[0,0,0,...], [0,7,0,...], ...]   // 24×24 array, values 0–7
```

### Dual-face frame (recto + verso)
```json
{
  "recto": [[...], ...],   // 24×24 array
  "verso": [[...], ...]    // 24×24 array
}
```

Values outside the cross shape (the four corner 8×8 quadrants) are ignored by the renderer.

---

## Panel layout

The five 8×8 panels are mapped into a 24×24 grid:

```
          cols 8–15
      ┌──────────────┐
      │     TOP      │   rows  0–7,  cols  8–15
      └──────────────┘

 cols 0–7    cols 8–15    cols 16–23
┌───────────────────────────────────┐
│   LEFT   │   CENTER   │   RIGHT   │  rows 8–15
└───────────────────────────────────┘

         cols 8–15
      ┌──────────────┐
      │    BOTTOM    │   rows 16–23, cols 8–15
      └──────────────┘
```

The four corner quadrants (e.g. rows 0–7 cols 0–7) are zero-filled and ignored.

---

## Architecture diagram

### GPIO wiring (hardware target)

```
Raspberry Pi 5          Shift-register chain
──────────────          ────────────────────
GPIO 24  (R_OE)    →   Recto  Output enable  (active low)
GPIO 25  (R_LATCH) →   Recto  Latch
GPIO  8  (R_CLK)   →   Recto  Clock
GPIO  7  (R_DATA)  →   Recto  Data

GPIO 14  (V_OE)    →   Verso  Output enable  (active low)
GPIO 15  (V_LATCH) →   Verso  Latch
GPIO 18  (V_CLK)   →   Verso  Clock
GPIO 23  (V_DATA)  →   Verso  Data
```

Each frame is shifted out as 320 bits (5 panels × 64 pixels) through the following sequence:

1. `SORTIE_3` HIGH → enable output
2. For each of the 320 pixels: `SORTIE_4` ← value, then `SORTIE_2` pulse (clock)
3. `SORTIE_1` pulse → latch (apply frame)

---

## Demo / Screenshots

### Web application

![Demo web recto](./assets/demo_recto_web.png)

![Demo web verso](./assets/demo_verso_web.png)

## Renderer

![Render](./assets/render.jpg)
![Double face](./assets/double_face.mov)

## Sound visualizer
![Music](./assets/music.mov)