# ⚽ Soccer Engine

Welcome to the **Soccer Engine AI Challenge**! This is a lightweight, SDL2-based soccer simulation designed for students and developers to practice low-level C programming, game engine architecture, and autonomous agent logic.

## 📌 Project Overview

The engine simulates a 6v6 soccer match. The core physics, rendering, and entity management are provided. The project focuses on two main logic modules: the **Referee** (`engine/logic/referee.c`) and the **Coach** (`engine/logic/coach.c`).

### Tech Stack

* **Language:** C (C99)
* **Graphics:** SDL2, SDL_ttf, SDL_image
* **Physics:** Custom 2D vector-based kinematics
* **Build System:** CMake

---

## 🚀 Phase 1: The Referee (Game Rules)

In this phase, you will focus on `engine/logic/referee.c`. The Referee is responsible for validating player actions and enforcing the rules of the match.

**Your Objectives:**

1. **Enforce Talent Limits:** Implement `verify_talents` to ensure the total talent points of each player do not exceed `MAX_TALENT_PER_PLAYER` (set to 20), while individual skills remain within the allowed range.
2. **Validate Movement:** Implement `verify_movement` to ensure players don't exceed the speed allowed by their Agility talent.
3. **Ball Possession Rules:** Validate that a player actually has the ball before they are allowed to enter the `SHOOTING` state.
4. **Set-Pieces:** Ensure that during a kickoff, the ball is passed backwards into the team's own half, as enforced by the restart rules.

**Goal:** Familiarize yourself with the `Scene` structure, the `Player` entity, and how the game loop interacts with the logic layer.

---

## 🧠 Phase 2: The Coach (AI Strategy)

Once the rules are enforced, it’s time to make the players smarter. You will work primarily in `engine/logic/coach.c`.

**Your Objectives:**

* **Role-Based Logic:** Assign different movement patterns to Defenders, Midfielders, and Attackers based on their `kit` number.
* **Intercepting:** Write logic so that if a ball comes within a certain radius, players automatically switch to the `INTERCEPTING` state to challenge for it.
* **Teamwork (Passing):** Players should scan for teammates and choose useful passing options instead of always attempting to shoot at the goal.
* **Defensive Positioning:** Defenders should stay between the ball and their own goal rather than simply chasing the ball.

**Goal:** Develop a rational AI agent that can compete effectively while staying within the rules enforced by the Referee.

---

## 🛠️ Installation & Setup

### Prerequisites

* **GCC** or **Clang** compiler
* **CMake 3.20+**
* **SDL2** libraries (including `SDL_ttf` and `SDL_image`)
* **xxd** (required for embedding `DejaVuSans.ttf` during the CMake build)

### Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

The executable is generated in:

```text
build/bin/soccerengine.exe
```

Or you can run run.bat in terminal:

```text
cd build/bin
./run.bat
```

On Windows, make sure `xxd` is available in your `PATH` before running the build.

---

## 📂 Project Structure

* `engine/core/`: Constants and Vector Math (`vec2`).
* `engine/entities/`: Definitions for `Ball`, `Player`, and `Team`.
* `engine/game/`: Match state and possession management.
* `engine/logic/`: Game rules and AI logic, including `referee.c` and `coach.c`.
* `engine/graphics/`: SDL2 renderer and scene management.
* `main.c`: Program entry point.

---

## 👨‍💻 Student / Developer

| **Name**       | Danial Roodsaraby (Draccoon) |
| -------------- | ----------------------------- |
| **Student ID** | 404105886                      |
| **GitHub**     | [github.com/danielroods](https://github.com/danielroods) |

---

## 🤝 Contributing

This project is based on the **Soccer Engine AI Challenge** provided by [Matin B02](https://github.com/MatinB02) and [Mani Ebrahimi](https://github.com/maniebra).

The repository contains Danial Roodsaraby's (Draccoon) implementation and improvements to the Coach AI and Referee logic as part of the project work.

Contributions are welcome. You can fork the repository, experiment with your own AI strategies or rule improvements, and open a Pull Request. Please keep changes focused, preserve the existing game architecture where possible, and avoid unnecessary rewrites of unrelated parts of the engine.

---

*Good luck, Coach! Bring home the trophy.* 🏆
