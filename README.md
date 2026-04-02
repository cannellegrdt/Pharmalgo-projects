# Pharmalgo - Pharmacy cross Hackathon

> Project developed during an **EPITECH hackathon**, with the goal of controlling a real physical LED pharmacy cross.

---

## The project

A **pharmacy cross** consists of **five 8×8 panels** arranged in a **+** shape:

```
         [  TOP  ]
[ LEFT ][ CENTER ][ RIGHT ]
        [ BOTTOM ]
```

Each LED supports a single brightness level. The full grid measures **24×24** pixels.

All programs communicate via the same protocol: **JSON sent via UDP on port 1337**. This allows for the seamless substitution of the simulator, editor, audio visualizer, or target hardware without modifying the core logic code.

---

## Simulators

### `Simulator/` - Pygame simulator (Python)

*Created by: Gabriel Destremont ([@Piguite](https://github.com/Piguite))*

`sim.py` opens a **Pygame** window and listens for UDP frames on port **1337**. It displays the 24×24 grid in real-time, drawing only the 5 valid panels (the corners of the grid are ignored). This is the primary development environment: it launches with a single command and faithfully simulates the physical rendering.

```bash
python3 Simulator/sim.py
```

### `SimulatorLGPIO/` - C++ Hardware driver (Raspberry Pi 5)

*Created by: Cannelle GOURDET ([@cannellegrdt](https://github.com/cannellegrdt))*


A C++ driver that receives the same UDP frames and translates them into GPIO signals to drive the **chain of shift registers** wired to the physical cross. It utilizes the **lgpio** library (`liblgpio-dev`). One frame = 320 bits (5 panels × 64 pixels) shifted onto the dedicated GPIO pins.

```bash
# On the Raspberry Pi 5
cd SimulatorLGPIO
make
sudo ./challenge_hw
```

> Both are **interchangeable**: same UDP protocol, zero changes required on the producer side.

---

## Projects

### `DrimeLogo/` - Logo animation

*Created by:*
 * Lucas Mageux ([@Lucas-DevL](https://github.com/Lucas-DevL))
 * Clément Pellegry ([@Clempllg](https://github.com/Clempllg))
 * Hugo Mathurin ([@hugomath133](https://github.com/hugomath133))

A geometric animation running at 30 fps, displaying shapes (inverted D, triangles, etc.) in a continuous loop-ideal for a presentation or a splash screen.

---

### `GameOfLife/` - Game of Life

*Created by:*
 * Lucas Mageux ([@Lucas-DevL](https://github.com/Lucas-DevL))
 * Clément Pellegry ([@Clempllg](https://github.com/Clempllg))
 * Hugo Mathurin ([@hugomath133](https://github.com/hugomath133))

An implementation of Conway's cellular automaton on the 24×24 grid, rendered in C++ at 10 fps. The simulation starts with a random state and evolves according to classic rules.

---

### `Lib_Croix/` - Shared library + utilities

*Created by:*
 * Gabriel Destremont ([@Piguite](https://github.com/Piguite))

A common C++ library (`CroixPharma.h/.cpp`) used by projects compiled for hardware (WiringPi). It also contains two standalone utilities:
- **`annonce.cpp`**: text scrolling with an integrated bitmap font.
- **`date.cpp`**: a real-time clock displaying the time on the panels.

---

### `Matrix/` - Matrix rain effect

*Created by:*
 * Cannelle GOURDET ([@cannellegrdt](https://github.com/cannellegrdt))

A shower of glowing characters falling down each column of the cross, featuring a bright head (level 7) and a fading tail (7 → 1). An infinite effect written in Python.

---

### `RealTimeEditor/` - Online Pixel-Art dditor

*Created by:*
 * Cannelle GOURDET ([@cannellegrdt](https://github.com/cannellegrdt))

A web-based editor inspired by Piskel. Draw directly onto the 5 panels from your browser, choose the brightness (0–7), and every modification is sent in real-time to the simulator via UDP (auto-send with a 50ms debounce). Note: the brightness level is only visible on the interface, not on the hardware. A Flask server acts as a relay between the JS canvas and the simulator.

```bash
cd RealTimeEditor && python3 server.py   # then open http://localhost:8080
```

---

### `SimonSays/` - Simon Says

*Created by:*
 * Tom Larconnier ([@tom-larconnier](https://github.com/tom-larconnier))
 * Ilan Leroux Pinchinat ([@Ilan-LP](https://github.com/Ilan-LP))
 * Khalil Almwakeh ([@volcinator8000](https://github.com/volcinator8000))

An interactive memory game. The cross lights up a directional panel (up/down/left/right) and plays a corresponding sound; the player must reproduce the sequence using the keyboard. Up to 10 rounds, with one extra life. `.wav` sounds included.

---

### `SoundVisualizer/` - Real-time audio visualizer

*Created by:*
 * Cannelle GOURDET ([@cannellegrdt](https://github.com/cannellegrdt))

Captures microphone input and displays an equalizer on the cross. Each panel covers a different frequency range (bass → left, mids → bottom, highs → top), featuring 8 bars per panel growing outwards. Uses `sounddevice` and `numpy` for FFT (Fast Fourier Transform).

---

### `Tetris/` - Tetris

*Created by:*
 * Hilaris Ferrari ([@ByyLafe](https://github.com/ByyLafe))
 * Lemi Dink ([@lemidink-debug](https://github.com/lemidink-debug))
 * Guillaume Nazaret ([@Guillaume256](https://github.com/Guillaume256))
 * Tom Larconnier ([@tom-larconnier](https://github.com/tom-larconnier))
 * Khalil Almwakeh ([@volcinator8000](https://github.com/volcinator8000))
 * Ilan Leroux Pinchinat ([@Ilan-LP](https://github.com/Ilan-LP))

A playable Tetris running on the central column of the cross (8×8 pixels wide). Pieces fall automatically and are controlled via the keyboard (z/q/s/d). Completed rows are cleared.

---

### `VideoPlayer/` - Video player

*Created by:*
 * Hugo Arnal ([@hugoarnal](https://github.com/hugoarnal))
 * Esteban Hazanas ([@kukutheaxolotl](https://github.com/kukutheaxolotl))
 * Sacha Defossez ([@sachadfz](https://github.com/sachadfz))
 * Hector Cordat ([@HectorCordat](https://github.com/HectorCordat))
 * Alexandre Kubiaczyk ([@Az0xI](https://github.com/Az0xI))
 * Nadal Berthelon ([@Nadal-B](https://github.com/Nadal-B))


Converts any video into frames tailored for the cross using FFmpeg and ImageMagick (`converter.sh`), then plays them in C++. Converted frames are stored in the `.pharma` format.

---

### `Warriors/` - Text scrolling

*Created by:*
 * Jonas Pustay ([@JonasPustay](https://github.com/JonasPustay))
 * Fatima Benzaoui ([@fatimabenzaoui](https://github.com/fatimabenzaoui))

Displays a scrolling text message across the panels of the cross. Includes a mini bitmap font engine in C++ to encode letters across 3 or 5 columns.

---


## Assets & Demo

...
