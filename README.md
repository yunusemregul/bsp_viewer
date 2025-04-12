# BSP Viewer

A 3D Source Engine BSP map file viewer built with OpenGL 4. This project explores and implements the Source Engine BSP file format to visualize maps with a custom OpenGL renderer.

![sample screenshot](https://i.ibb.co/gJKZBr4/image.png)

## Features

- Load and parse Source Engine BSP file format
- 3D visualization of BSP maps with OpenGL 4
- First-person camera controls with keyboard and mouse
- Support for textures and transparency
- Handling of face geometry and basic displacement mapping

## Implementation Details

- BSP parsing of lumps including: vertices, edges, planes, faces, models, etc.
- OpenGL shader-based rendering pipeline
- Keyboard/mouse camera controls with zoom functionality

## Included Sample Maps

The project comes with several sample BSP maps:
- `sphere.bsp`
- `disp_center_height_4.bsp`
- `cube.bsp`

## Building

```
make build
```

## Usage

```
./bsp-viewer maps/cube.bsp
```

## Controls

- WASD/Arrow Keys: Move camera
- Mouse: Look around
- Scroll Wheel: Zoom in/out
- Space: Move up
- Shift: Speed up movement
- Ctrl: Slow down movement

## Sources
* Understanding the BSP file format
  * https://developer.valvesoftware.com/wiki/Source_BSP_File_Format
* Camera controller and base OpenGL code
  * https://learnopengl.com/
