# Mandelbrot Explorer

## Features

- Intuitive zoom and pan behavior
- Ability to change float point precision
- Ability to change maximum iterations

## Requirements

- Windows 10
- Visual Studio 2022
  - Including "Desktop Development with C++"

May work newer versions of Windows or Visual Studio, I have not checked.

## Usage

Open the .sln file in Visual Studio 2022 and click the green play button at the top of the window near the navbar.

## Controls

Pressing the following buttons in the window have the following actions:

- "X" toggles between double and single precision (double is significantly slower). Program starts in single precision mode.
- "A" toggles automatic switch between single and double precision. Switch happens at x10000 zoom
- "R" resets zoom and pan
- `space` prints zoom and pan info to the console
- `up/down arrow` increases/decreases the maximum iterations done in the mandelbrot computation

Pan around by clicking, holding and dragging. \
Zoom in and out with the scroll wheel.

## Benchmark

Running on a GTX 1060 6GB and i7-7700k at 1080p:

- Single floating point precision at 1000 iterations:
  - in regions in which max-iterations are reached across the entire screen 60 fps
  - in an average area at 144+ fps
- Double floating point precision:
  - in regions in which max-iterations are reached across the entire screen 4 fps
  - in an average area at ~20 fps

Reducing window size and max-iterations improves fps significantly.
