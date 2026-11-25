# PAPERLAND - Chapter 1: The Enchanted Forest

## Build
**Windows (MinGW / MSYS2):** double-click `build.bat`, or run  
`g++ -std=c++11 -O2 -Iinclude src\core\*.cpp src\shared\*.cpp src\chapters\chapter1\*.cpp -o PAPERLAND.exe -lfreeglut -lopengl32 -lglu32`  
**Linux:** `make` (needs freeglut: `sudo apt install freeglut3-dev`)

> The old `g++ src\*.cpp` command no longer works: sources are now in sub-folders
> and headers are found through `-Iinclude`.
> `backup/` holds your old `animation_backup.cpp` on purpose; do not compile it.

## Folder structure
```
include/ and src/ mirror each other
  core/                 main.cpp, menu, game.cpp (chapter registry + routing), chapter.h (interface)
  shared/               used by every chapter: paper toolkit, particles, audio, book animation
  chapters/chapter1/    The Enchanted Forest - everything specific to Chapter 1
  chapters/chapter2-4/  empty, ready for the next chapters
```
Chapter 1 is reached only through `chapters/chapter1/chapter1.h`
(`startChapter1, updateChapter1, drawChapter1, handleChapter1Mouse / MouseMove /
Keyboard / SpecialKey, isChapter1Complete`). Chapter 1 does not include anything
from other chapters; `main.cpp` and the book only talk to `core/game`.

**Adding Chapter 2:** create `src/chapters/chapter2/` + `include/chapters/chapter2/`,
expose `const Chapter& chapter2()` (copy the table at the end of `chapter1.cpp`),
register it in `game::init()` (`src/core/game.cpp`), and add its folder to `build.bat`.
The Makefile picks new folders up automatically.

## Controls
| Key / input | Action |
|---|---|
| Mouse click | Interact with creatures, objects, hidden treasures |
| SPACE | Pause / resume |
| H | Hint rings on undiscovered treasures |
| S | Skip the unfolding / skip the story |
| ENTER or N | Start the story now / next dialogue |
| T | Replay the story (after the forest is ready) |
| R | Restart Chapter 1 |
| M | Mute / unmute sound |
| ESC | Back to menu, Q = quit |
| LEFT / RIGHT arrows | Chapter navigation messages (Ch. 2-3 not built yet); RIGHT = next dialogue in the story |

## Flow
1. **Book opens** - the cover flips over the spine.
2. **Unfolding (7 stages, ~12 s)** - landscape, trees, water, village, inhabitants, magic, alive.
3. **Exploration** - click everything, find 5 hidden treasures (counter top-right).
   The story starts when all 5 are found (after a short message) or after 55 s,
   or when you press ENTER.
4. **Cinematic** - page dims, magic particles, "Once upon a time", then 5 illustrated
   scenes with camera movement and typewriter dialogue (auto-advances; click / ENTER to speed up).
5. **After the story** - forest restored, clicks re-enabled, BACK TO BOOK pulses and closes the book.

## Modules
| File | Responsibility |
|---|---|
| `core/main.cpp` | window, GLUT callbacks, single 16 ms frame timer |
| `core/menu.cpp` | your original menu |
| `core/game.cpp` | chapter registry; routes input / update / draw to the active chapter |
| `shared/book_animation.cpp` | open book: cover flip, pages, closing (chapter-agnostic) |
| `shared/paper.cpp` | paper-cut toolkit (shapes + offset shadows, easing, text, time) |
| `shared/particles.cpp` | particle pool (sparkles, motes, leaves, drops, mist, coins) |
| `shared/audio.cpp` | synthesized sound effects + lullaby (Windows waveOut; silent elsewhere) |
| `chapters/chapter1/chapter1.cpp` | Chapter 1 director: 7-stage timeline, phases, input |
| `.../forest_environment.cpp` | sky, sun, clouds, mountains, hills, ground (+ init/update) |
| `.../forest_trees.cpp` | trees: unfold, click, hover |
| `.../forest_water.cpp` | waterfall, river, bridge, pond, path stones |
| `.../forest_characters.cpp` | Flick the fox, Pip the rabbit, the deer, butterflies, birds |
| `.../forest_objects.cpp` | mushroom houses, Ancient Tree, chest, flowers, mushrooms, discovery icons |
| `.../forest_effects.cpp` | fireflies, ripples, falling leaves, waterfall spray |
| `.../forest_interactions.cpp` | click dispatch, 5 discoveries, messages, hover, HUD, back button |
| `.../forest_story.cpp` | cinematic: transition, 5 scenes, camera, dialogue |
| `.../forest_layout.h` | Chapter 1 coordinates and constants |

Camera design: **Option A** - fixed book view while exploring, camera movement (pan/zoom) only
inside the cinematic.

## Notes
* Sound needs no files. It is generated at start-up and plays on Windows only.
  `winmm.dll` is loaded at run time, so no extra linker flag is required.
* The window can be resized; the scene scales with it.
