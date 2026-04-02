Fin les gars la lib est MISERABLE (compiler juste le main avec CroixPharma)
*Installation Lib GPIO raspi :**

```bash
sudo apt update
sudo apt install git build-essential

git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build
gpio -v
```

**Compilation :**
```bash
g++ main.cpp CroixPharma.cpp -lwiringPi -o programme
```

---
