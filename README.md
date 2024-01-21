GLEW and GLFW setup using this video: <https://www.youtube.com/watch?v=vGptI11wRxE>

Important: in VS2022 in the navbar under Project -> \<project_name\> Properties you need to edit `C/C++ -> General -> Additional Include Directories`, `Linker -> General -> Additional Library Directories`, `Linker -> Input -> Additional Dependencies`

## Controls

Pressing the following buttons in the window have the following actions:

- "X" toggles between double and single precision (double is significantly slower). Program starts in single precision mode.
- "A" toggles automatic switch between single and double precision. Switch happens at x10000 zoom
- "R" resets zoom and pan
- `space` prints zoom and pan info to the console
- `up/down arrow` increases/decreases the maximum iterations done in the mandelbrot computation

Pan around by clicking, holding and dragging. \
Zoom in and out with the scroll wheel.
