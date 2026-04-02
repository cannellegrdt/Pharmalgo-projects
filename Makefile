PROJECTS = DrimeLogo GameOfLife Matrix RealTimeEditor SimonSays SoundVisualizer Tetris VideoPlayer Warriors
BINARIES = drimelogo gameoflife simonsays tetris videoplayer warriors simulatorlgpio


all:
	./run.sh

$(PROJECTS):
	./run.sh $@

install: check-wiringpi check-lgpio venv-install
	@echo ""
	@echo "Installation done."

venv-install:
	@if [ ! -d .venv ]; then \
		echo "[venv] Creating the virtual environment..."; \
		python3 -m venv .venv; \
	else \
		echo "[venv] Existing virtual environment."; \
	fi
	@echo "[pip] Installation of dependencies..."
	@.venv/bin/pip install -r RealTimeEditor/requirements.txt --quiet
	@.venv/bin/pip install -r SoundVisualizer/requirements.txt --quiet
	@echo "[pip] OK"

check-wiringpi:
	@echo "[wiringPi] Verification..."
	@if ldconfig -p | grep -q "libwiringPi"; then \
		echo "[wiringPi] OK"; \
	else \
		echo "[wiringPi] Not found. Installation from WiringPi/..."; \
		cd WiringPi && ./build; \
	fi

check-lgpio:
	@echo "[lgpio] Verification..."
	@if ldconfig -p | grep -q "liblgpio"; then \
		echo "[lgpio] OK"; \
	else \
		echo "[lgpio] Not found. Install it via :"; \
		echo "  sudo apt install liblgpio-dev"; \
		exit 1; \
	fi

clean:
	rm -f $(BINARIES)

fclean: clean
	rm -rf .venv

.PHONY: all install clean fclean $(PROJECTS)
