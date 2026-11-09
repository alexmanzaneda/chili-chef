# Improvements

Checklist of what is planned for the game. Tick an item (`[x]`) in the same commit that does it.

## Next

- [ ] **Automatic pause when leaving the game**
  If the page goes to the background (another app, a call, another tab) or the window loses focus,
  the game opens the pause menu, so it does not go on, or kill the chef, the moment you come back.
  - Web: on `visibilitychange` / `blur` the page tells the game to pause (like the touch buttons do).
  - Desktop: the same with the focus callback of GLFW (`glfwSetWindowFocusCallback`).
  - Only during a level (`GameScene`); the menus and the other screens do not need it.
  - Done when: switching apps on the phone and coming back shows the pause menu.

- [ ] **Save the progress**
  Reloading the page or closing the game starts from zero now.
  - To decide: what is saved. The highest level reached at least (to continue from it);
    maybe the lives and power-ups it was reached with, and the best time of each level.
  - Desktop: a small file next to the executable. Web: the browser storage (`localStorage`,
    from C++ with `EM_ASM`), since files written in the web version are lost when it reloads.
  - The menu shows a CONTINUE option only when there is something saved.
  - Done when: after clearing level 1-1 and reloading the page, CONTINUE starts at 1-2.

## Later

- [ ] **Fullscreen on mobile**: a button that hides the browser bars (Fullscreen API, Android).
  On iPhone it only works by adding the game to the home screen, which needs a web manifest.
- [ ] **Smaller download**: the WAV music is ~4.6 MB of the ~8 MB. Another format (OGG) would load
  much faster on a phone, as long as the music still loops without a jump.
- [ ] **Try the mobile zoom on a real phone** and adjust `MOBILE_ZOOM` (`Game.h`) and the size of the
  touch buttons (`web/shell.html`) if needed.

## Done

- [x] Web version on GitHub Pages
- [x] Touch controls for mobile
- [x] Camera zoom on mobile
- [x] Custom loading screen: percentage and the chef walking on a bar
