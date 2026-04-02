##
## EPITECH PROJECT, 2025
## Pharmalgo
## File description:
## server.py
##

#!/usr/bin/env python3
"""
Pharmacy Cross LED Editor - Backend Server

Serves the web interface and relays frame data to the UDP simulator (sim.py).
Supports independent recto/verso face control.
"""

from flask import Flask, request, jsonify, send_from_directory
import socket
import json

app = Flask(__name__, static_folder="static")

UDP_HOST = "127.0.0.1"
UDP_PORT = 1337

PANEL_OFFSETS = {
    "top": (0, 8),
    "left": (8, 0),
    "center": (8, 8),
    "right": (8, 16),
    "bottom": (16, 8),
}

current_frames = {
    "recto": [[0] * 24 for _ in range(24)],
    "verso": [[0] * 24 for _ in range(24)],
}


def panels_to_frame(panels: dict) -> list:
    """Convert 5 panel grids (each 8x8, values 0-7) to the 24x24 array expected by sim.py."""
    frame = [[0] * 24 for _ in range(24)]
    for panel_name, (row_off, col_off) in PANEL_OFFSETS.items():
        panel = panels.get(panel_name, [])
        for row in range(8):
            panel_row = panel[row] if row < len(panel) else []
            for col in range(8):
                val = panel_row[col] if col < len(panel_row) else 0
                frame[row_off + row][col_off + col] = int(val)
    return frame


def send_udp(recto: list, verso: list):
    """Send both faces as a dual-frame JSON object to the hardware/simulator."""
    payload = json.dumps({"recto": recto, "verso": verso}).encode("utf-8")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(payload, (UDP_HOST, UDP_PORT))
    finally:
        sock.close()


@app.route("/")
def index():
    return send_from_directory("static", "index.html")


@app.route("/api/frame", methods=["POST"])
def api_frame():
    """
    Accept a frame for one face.
    Expected body: { "face": "recto"|"verso", "panels": { top, left, center, right, bottom } }
    Defaults to "recto" when the face field is absent (backwards compatible).
    """
    data = request.get_json(force=True, silent=True) or {}
    face = data.get("face", "recto")
    panels = data.get("panels", {})
    frame = panels_to_frame(panels)

    if face in ("recto", "verso"):
        current_frames[face] = frame
    try:
        send_udp(current_frames["recto"], current_frames["verso"])
        return jsonify({"status": "ok"})
    except Exception as exc:
        return jsonify({"status": "error", "message": str(exc)}), 500


@app.route("/api/frame/both", methods=["POST"])
def api_frame_both():
    """
    Update both faces in a single request.
    Expected body: { "faces": { "recto": { panels… }, "verso": { panels… } } }
    """
    data = request.get_json(force=True, silent=True) or {}
    faces = data.get("faces", {})
    for face_name in ("recto", "verso"):
        panels = faces.get(face_name, {})
        current_frames[face_name] = panels_to_frame(panels)
    try:
        send_udp(current_frames["recto"], current_frames["verso"])
        return jsonify({"status": "ok"})
    except Exception as exc:
        return jsonify({"status": "error", "message": str(exc)}), 500


@app.route("/api/clear", methods=["POST"])
def api_clear():
    """
    Clear one or both faces.
    Optional body: { "face": "recto"|"verso"|"both" }  — defaults to "both".
    """
    data = request.get_json(force=True, silent=True) or {}
    face = data.get("face", "both")

    if face in ("recto", "both"):
        current_frames["recto"] = [[0] * 24 for _ in range(24)]
    if face in ("verso", "both"):
        current_frames["verso"] = [[0] * 24 for _ in range(24)]
    try:
        send_udp(current_frames["recto"], current_frames["verso"])
        return jsonify({"status": "ok"})
    except Exception as exc:
        return jsonify({"status": "error", "message": str(exc)}), 500


if __name__ == "__main__":
    print("=" * 50)
    print("  Pharmacy Cross LED Editor")
    print("  http://localhost:8080")
    print(f"  Relaying frames → UDP {UDP_HOST}:{UDP_PORT}")
    print("=" * 50)
    app.run(debug=False, host='0.0.0.0', port=8080)
