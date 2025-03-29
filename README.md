# Shapes War 🔺🟦💥

🚧 **Work in Progress**  
Shapes War is still under active development! New features and improvements are being added regularly — this is not the final version. Expect more weapons, enemy types, power-ups, and visual polish in upcoming updates.

---

## 🎮 Gameplay

- Move using `W`, `A`, `S`, `D`
- Shoot bullets with **left click**
- Use a special laser weapon with **right click**
- Press **Space** to pause
- Avoid collisions with enemies and survive as long as possible

Enemies spawn over time at a random position and they may split into smaller ones when destroyed. The special laser weapon pierces through all enemies in a vertical line.

---

## 🛠️ Built With

- **C++17**
- **[SFML 3.0](https://www.sfml-dev.org/)** - for graphics, input, and window handling
- Object-oriented design using a custom **Entity-Component System**

---

## 🔧 Configuration

The game reads its settings from a plain text `config.txt` file, which defines:

- Window size, framerate, and fullscreen mode
- Font, text size, and color
- Player stats (speed, shape, color)
- Enemy stats (spawn interval, speed, shape, etc.)
- Bullet stats (speed, color, lifetime)

Example `config.txt`:



---

## ▶️ How to Run

1. Install **SFML 3.0**
2. Clone this repo
3. Compile:
   ```bash
   g++ -std=c++17 -o shapeswar main.cpp Game.cpp Entity.cpp EntityManager.cpp Vec2.cpp -lsfml-graphics -lsfml-window -lsfml-system

