# Chili Chef

Juego 2D estilo Game Boy hecho en C++ con OpenGL (asignatura VJ, UPC).

## Compilar y ejecutar

1. Abrir `game/chili-chef.sln` con Visual Studio (Archivo → Abrir → Proyecto o solución; no "Abrir carpeta").
2. Configuración `Debug|Win32` o `Release|Win32` y pulsar F5.

`glew32.dll` se copia automáticamente junto al ejecutable al compilar.

## Estructura

```
game/       Solución de Visual Studio
  src/      Código fuente (.cpp/.h)
  images/   Texturas que carga el juego
  levels/   Mapas de los niveles
  shaders/  Shaders GLSL
  sounds/   Efectos de sonido (.wav, generados con tools/make_sounds.py)
  music/    Música de fondo (.wav, generada con tools/make_music.py)
libs/       Librerías externas (GLFW, GLEW, GLM, SOIL, FreeType, miniaudio)
art/        Arte fuente (.xcf, mapas de Tiled .tmx/.tsx)
tools/      Scripts que generan imágenes, niveles, sonidos y música
docs/       Enunciado de la práctica
```

## Audio

El audio usa [miniaudio](https://miniaud.io) (un único header en `libs/`, dominio público, sin DLLs).
Todo pasa por la clase `Audio` (`src/Audio.h`):

```cpp
Audio::instance().playSound(SOUND_EXPLOSION); // Efecto corto, se pueden solapar
Audio::instance().playMusic(MUSIC_KITCHEN);   // Pista de fondo en bucle, solo una a la vez
```

- **Añadir un efecto:** su id en `SoundId` (`Audio.h`), su fichero en `SOUND_FILES` (`Audio.cpp`)
  y su función en `tools/make_sounds.py` (o un .wav propio en `game/sounds/`).
- **Música:** los ficheros de `MUSIC_FILES` (`game/music/menu.wav`, `kitchen.wav`, `fridge.wav`).
  `tools/make_music.py` los genera a partir de notas escritas en el script (provisionales); para
  cambiar uno basta con poner otro .wav con el mismo nombre (por ejemplo, exportado de BeepBox).
  En WAV y no MP3: el MP3 lleva silencio al principio y al final y el bucle da un salto.
  Cada mundo elige su pista en `Levels.h`. Si falta un fichero el juego lo avisa por consola y sigue sin él.
