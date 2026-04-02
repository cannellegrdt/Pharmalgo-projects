# Pharmalgo LED Editor

Real-time pixel editor for pharmacy crosses, inspired by Piskel.
Draw directly onto the 5 8×8 panels of the cross and send each frame to the Python simulator in real-time via UDP.

---

## Folder structure

```
editor/
├── server.py            # Flask Server: web interface + UDP relay to sim.py
├── requirements.txt     # Python dependency (Flask)
├── static/
│   ├── index.html       # Main page
│   ├── style.css        # Dark theme
│   └── editor.js        # Editor logic (canvas, tools, networking)
└── README.md            # This file
```

---

## Prerequisites

- Python 3.8+
- pip

---

## Installation

```bash
cd editor
pip install -r requirements.txt
```

---

## Running the application

### 1 — Start the simulator (from the `Simulator/` folder)

```bash
python3 sim.py
```

The Pygame window will open and listen on UDP port 1337.

### 2 — Start the editor (from the `editor/` folder)

```bash
python3 server.py
```

Then open **http://localhost:8080** in your browser.

---

## Interface

```
┌─────────────────────────────────────────────────────────────┐
│  +  Pharmacy Cross LED Editor      ● Sent    [Auto-send ▶]  │
├──────┬──────────────────────────────────────────────────────┤
│Tools │                                          │ Sidebar   │
│      │                TOP                       │           │
│  ✏   │         LEFT  CENTER  RIGHT              │Brightness │
│  ⬜  │               BOTTOM                     │           │
│  🪣  │                                          │ Slider    │
│  👁  │                                           │          │
│      │                                          │ Shortcuts │
│Bright.│                                         │           │
└──────┴──────────────────────────────────────────┴───────────┘
```

---

## Tools

| Tool       | Shortcut | Description                                   |
|------------|----------|-----------------------------------------------|
| Pencil     | `P`      | Draw with the selected brightness             |
| Eraser     | `E`      | Erase a pixel (brightness → 0)                |
| Fill       | `F`      | Flood fill tool                               |
| Eyedropper | `I`      | Pick the brightness from an existing LED      |

---

## Brightness

The LEDs support **8 levels**: `0` (off) to `7` (maximum brightness).

| Method                  | Description                                  |
|--------------------------|----------------------------------------------|
| Keys `0` to `7`          | Change brightness directly via keyboard      |
| Slider (right sidebar)   | Sliding cursor                               |
| Swatches (left bar)      | 8 clickable swatches with visual preview     |

---

## Keyboard shortcuts

| Key           | Action                     |
|---------------|----------------------------|
| `P`           | Pencil Tool                |
| `E`           | Eraser Tool                |
| `F`           | Fill Tool (bucket)         |
| `I`           | Eyedropper Tool (picker)   |
| `0` – `7`     | Set brightness level       |
| `S`           | Send frame manually        |
| `Delete`      | Clear all panels           |

---

## Sending to simulator

- **Auto-send** (enabled by default): every modification is automatically sent to the simulator, with a 50ms debounce delay to group rapid actions.
- **"Send Frame" Button**: manual send at any time.
- The status indicator at the top displays: *Sent* (green), *Sending…* (blue), *Simulator not running* (red, the editor continues to function normally).

---

## Technical architecture

```
Browser
  │  drawing on JS canvas
  │  POST /api/frame  (JSON)
  ▼
server.py  (Flask, port 5000)
  │  converts 5×(8×8) → 24×24 grid
  │  UDP JSON → port 1337
  ▼
sim.py  (Pygame)
  │  receives the 24×24 grid
  │  displays LEDs in real-time
  ▼
(Future: actual physical pharmacy cross)
```

### Panel mapping within the 24×24 grid

```
         cols 8-15
    ┌──────────────┐
row │     TOP      │  rows 0-7,  cols 8-15
0-7 └──────────────┘

     cols 0-7    cols 8-15   cols 16-23
    ┌───────────────────────────────────┐
row │   LEFT   │   CENTER   │   RIGHT   │  rows 8-15
8-15└───────────────────────────────────┘

         cols 8-15
     ┌──────────────┐
row  │    BOTTOM    │  rows 16-23, cols 8-15
16-23└──────────────┘
```

Zeros in unused positions (the corners of the 24×24 grid) are ignored by `sim.py`.
