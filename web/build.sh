#!/usr/bin/env bash
# Builds the web version of the game (WebAssembly + WebGL 2) into dist/.
# Needs Emscripten (emcc) in the PATH: https://emscripten.org/docs/getting_started/downloads.html
# GitHub Actions runs it on every push to main (.github/workflows/pages.yml).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/dist"
SOIL="$ROOT/libs/Simple OpenGL Image Library/src"
mkdir -p "$OUT"
cd "$ROOT/game"

# From SOIL only its image loader (stb_image, in C) is needed: see Texture.cpp
emcc -c -O2 "$SOIL/stb_image_aug.c" -o "$OUT/stb_image_aug.o"

# contrib.glfw3 is a GLFW for the browser that also reads gamepads (the Gamepad API).
# The game opens its files with relative paths (images/..., levels/...): --preload-file
# packs them into index.data and puts them at those same paths
em++ -O2 -std=c++17 src/*.cpp "$OUT"/*.o \
	-I"$SOIL" -I"$ROOT/libs/miniaudio-0.11.22" -I"$ROOT/libs/glm" \
	--use-port=contrib.glfw3 \
	-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
	-sALLOW_MEMORY_GROWTH=1 \
	--preload-file images --preload-file levels --preload-file shaders \
	--preload-file sounds --preload-file music \
	--shell-file "$ROOT/web/shell.html" \
	-o "$OUT/index.html"

rm -f "$OUT"/*.o
echo "Built $OUT/index.html"
