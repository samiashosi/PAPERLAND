# PAPERLAND - Chapter 1: The Enchanted Forest

## Build
**Windows (MinGW / MSYS2):** double-click `build.bat`, or run  
`g++ -std=c++11 -O2 src\*.cpp -o PAPERLAND.exe -lfreeglut -lopengl32 -lglu32`  
**Linux:** `make` (needs freeglut: `sudo apt install freeglut3-dev`)

> Compile **every** file in `src/` - Chapter 1 is split into modules.
> `backup/` holds your old `animation_backup.cpp` on purpose; do not compile it
> (it defines the same functions as `animation.cpp`).

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

## Modules (as in the Chapter 1 plan)
| File | Responsibility |
|---|---|
| `main.cpp` | window, GLUT callbacks, single 16 ms frame timer |
| `menu.cpp` | your original menu (kept; only the START/back/keyboard hooks changed) |
| `animation.cpp` | open book: cover flip, pages, closing animation |
| `forest.cpp` | Chapter 1 director: 7-stage timeline, scene order, phases, input |
| `environment.cpp` | sky, sun, clouds, mountains, hills, trees, river, waterfall, bridge, pond, paths |
| `characters.cpp` | Flick the fox, Pip the rabbit, the deer, butterflies, birds |
| `objects.cpp` | mushroom houses, Ancient Tree, treasure chest, flowers, mushrooms, discovery icons |
| `effects.cpp` | fireflies, sparkles, ripples, falling leaves, waterfall spray |
| `interactions.cpp` | click dispatch, 5 discoveries (0/5), messages, hover cursor, HUD, back button |
| `story.cpp` | cinematic: transition, 5 scenes, camera, dialogue |
| `paper.cpp` | shared paper-cut toolkit (shapes + offset shadows, easing, text, time) |
| `audio.cpp` | synthesized sound effects + lullaby (Windows waveOut; silent elsewhere) |

Camera design: **Option A** - fixed book view while exploring, camera movement (pan/zoom) only
inside the cinematic.

## Notes
* Sound needs no files. It is generated at start-up and plays on Windows only.
  `winmm.dll` is loaded at run time, so no extra linker flag is required.
* The window can be resized; the scene scales with it.
