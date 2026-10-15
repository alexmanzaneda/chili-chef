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
libs/       Librerías externas (GLFW, GLEW, GLM, SOIL, FreeType)
art/        Arte fuente (.xcf, mapas de Tiled .tmx/.tsx)
docs/       Enunciado de la práctica
```
