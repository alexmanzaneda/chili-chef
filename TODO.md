# Improvements

Checklist of what is planned for the game. Tick an item (`[x]`) in the same commit that does it.

## Next

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
- [x] Automatic pause when the window, tab or app loses the focus
