#  Turn-Based Combat Mod for Quake II

This is a custom Quake II mod that transforms the original real-time gameplay into a **turn-based tactical experience**, inspired by strategy games like XCOM.

In this mod, you control a team of 5 player units, taking turns to move, shoot, and rotate, while facing off against 5 AI-controlled enemy units. Each side takes turns, one unit at a time, in a sequential loop — making every decision count.

---

##  How to Install & Play

### 1. **Download the Repository**
Clone or download the full repo:
git clone https://github.com/Marvinda0/quake2-full


### 2. **Build the Mod**
Open the project in Visual Studio and build the solution. Your modded `.dll` will compile into the `release` or `debug` folder.

### 3. **Setup Your Mod Folder**
- Create a new folder in your Quake II directory called `/turnbased` (or whatever name you want).
- Copy the compiled game DLL (`gamex86.dll`) into that folder.
- Include any required assets or `pak` files if needed (you probably only need the `.dll`).

### 4. **Launch the Mod**
Create a shortcut or use the console:


**Note:** The mod currently only works on the map `qdm1`. You must manually load it from the console after launching.

---

##  Controls

| Action        | Key       |
|---------------|-----------|
| End Turn      | `F3`      |
| Move Forward  | `V`       |
| Turn Left/Right/Back | Arrow Keys / `F4` |
| Shoot         | `F`       |

> Press `F1` to view the in-game help screen at any time.

---

##  How to Play

- Each team (Player & AI) takes turns.
- Only one unit acts at a time per team.
- During a unit's turn, you can move, shoot, and rotate.
- Once a unit finishes, the next unit on your team takes over.
- After all units have acted, the turn passes to the other team.

---

## ⚠ Known Issues

- The AI still uses Quake II’s original logic, so it may get stuck on stairs, walls, or ledges.
- Units may behave unpredictably on uneven terrain.
- The mod is only tested and functional on `q2dm1`.

---

## Completed Features

- Turn-based system with team cycling
- 5 player-controlled units + 5 enemy AI units
- In-game UI showing active unit and turn
- Help screen with instructions (`F1`)
- Third-person camera that follows the current unit
- Auto-launch shortcut and standalone mod folder

---

## 💬 Contact

Made by [@Marvinda0](https://github.com/Marvinda0)  
This was built as a final project for a game development course.

---





