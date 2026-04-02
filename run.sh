#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TIMEOUT=600  # 10 minutes

ALL_PROJECTS=("DrimeLogo" "Matrix" "VideoPlayer" "Warriors")
GROUP1=("DrimeLogo" "VideoPlayer" "Warriors") # w. Simulator/ and Lib_Croix/
GROUP2=("Matrix") # w. SimulatorLGPIO

SIMULATOR_PID=""
PROJECT_PID=""
TIMER_PID=""


cleanup() {
    echo ""
    echo "Stopping now..."
    [ -n "$TIMER_PID" ] && kill "$TIMER_PID" 2>/dev/null
    [ -n "$PROJECT_PID" ] && kill "$PROJECT_PID" 2>/dev/null && wait "$PROJECT_PID" 2>/dev/null
    [ -n "$SIMULATOR_PID" ] && kill "$SIMULATOR_PID" 2>/dev/null && wait "$SIMULATOR_PID" 2>/dev/null
    exit 0
}

trap cleanup SIGINT SIGTERM


is_in_group() {
    local project="$1"; shift
    local arr=("$@")
    for p in "${arr[@]}"; do [ "$p" = "$project" ] && return 0; done
    return 1
}

is_valid_project() {
    is_in_group "$1" "${ALL_PROJECTS[@]}"
}

compile_cpp() {
    local project="$1"
    echo "[Compilation] $project..."
    case "$project" in
        DrimeLogo) g++ DrimeLogo/main.cpp Lib_Croix/CroixPharma.cpp -lwiringPi -o drimelogo ;;
        GameOfLife) g++ GameOfLife/main.cpp Lib_Croix/CroixPharma.cpp -lwiringPi -o gameoflife ;;
        SimonSays) g++ SimonSays/main.cpp Lib_Croix/CroixPharma.cpp -lwiringPi -o simonsays ;;
        Tetris) g++ Tetris/main.cpp Lib_Croix/CroixPharma.cpp -lwiringPi -o tetris ;;
        VideoPlayer) g++ VideoPlayer/main.cpp Lib_Croix/CroixPharma.cpp -lwiringPi -o videoplayer ;;
        Warriors) g++ Warriors/main.cpp Lib_Croix/CroixPharma.cpp -lwiringPi -o warriors ;;
    esac
}

ensure_simulator() {
    if [ -n "$SIMULATOR_PID" ] && kill -0 "$SIMULATOR_PID" 2>/dev/null; then
        return
    fi

    if [ ! -f "$SCRIPT_DIR/simulatorlgpio" ]; then
        echo "[Compilation] SimulatorLGPIO..."
        g++ SimulatorLGPIO/*.cpp -llgpio -o simulatorlgpio || {
            echo "Error: SimulatorLGPIO compilation failed" >&2; exit 1
        }
    fi

    echo "[Lancement] SimulatorLGPIO..."
    ./simulatorlgpio &
    SIMULATOR_PID=$!
    sleep 1
}

ensure_venv() {
    if [ ! -d "$SCRIPT_DIR/.venv" ]; then
        echo "[Setup] Creating a Python venv..."
        python3 -m venv "$SCRIPT_DIR/.venv"
    fi

    if [ -f "./requirements.txt" ]; then
        echo "[Setup] Installation of dependencies : ./requirements.txt..."
        "$SCRIPT_DIR/.venv/bin/pip" install -r "./requirements.txt" --quiet
    fi
}


launch_project() {
    local project="$1"
    echo ""
    echo "=============================="
    echo "  Launch : $project"
    echo "=============================="

    if is_in_group "$project" "${GROUP1[@]}"; then
        compile_cpp "$project" || { echo "Error: $project's compilation failed" >&2; return 1; }
        local binary
        binary=$(echo "$project" | tr '[:upper:]' '[:lower:]')
        ./"$binary" &
        PROJECT_PID=$!

    elif is_in_group "$project" "${GROUP2[@]}"; then
        ensure_simulator

        case "$project" in
            Matrix)
                python3 Matrix/matrix.py &
                PROJECT_PID=$!
                ;;
            RealTimeEditor)
                ensure_venv
                "$SCRIPT_DIR/.venv/bin/python3" RealTimeEditor/server.py &
                PROJECT_PID=$!
                ;;
            SoundVisualizer)
                ensure_venv
                "$SCRIPT_DIR/.venv/bin/python3" SoundVisualizer/sound_visualizer.py &
                PROJECT_PID=$!
                ;;
        esac
    fi
}

run_with_timeout() {
    launch_project "$1" || return

    ( sleep "$TIMEOUT"; kill "$PROJECT_PID" 2>/dev/null ) &
    TIMER_PID=$!

    wait "$PROJECT_PID" 2>/dev/null || true

    kill "$TIMER_PID" 2>/dev/null
    wait "$TIMER_PID" 2>/dev/null || true

    PROJECT_PID=""
    TIMER_PID=""
}


if [ -n "$1" ]; then
    if ! is_valid_project "$1"; then
        echo "Error: unknown project '$1'" >&2
        echo "Available projects : ${ALL_PROJECTS[*]}" >&2
        exit 1
    fi
    launch_project "$1" || exit 1
    wait "$PROJECT_PID"

else
    echo "Loop mode — rotates every $((TIMEOUT / 60)) minutes"
    echo "Projects : ${ALL_PROJECTS[*]}"
    INDEX=0
    while true; do
        run_with_timeout "${ALL_PROJECTS[$INDEX]}"
        INDEX=$(( (INDEX + 1) % ${#ALL_PROJECTS[@]} ))
    done
fi
