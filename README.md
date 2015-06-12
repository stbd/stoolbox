# SToolbox
Storytelling Toolbox: a small collection of tools to get started with visual applications.

## What is this?
Well, first of all this is a hobby project (so be gentle). The idea of SToolbox is to be some kind of collection of tools that help you get started with visual applications on top of OpenGL, as well as a set of scripts that prepare a development environment by installing commonly used libraries. And, since it is a hobby project, it is a place to test and learn about software architectures and development.

## What's included
 * Scripts for setting up development enviroment on Linux and Windows
   * Meaning scripts for installing SDL2, Boost, GLM, GLLoadgen, and Freetype2
 * Wrapper for for holding vertex/attribute data
 * Wrapper for creating and using OpenGL shader
 * Wrapper for creating and using Vertex Array Object
 * Functions for generating few basic geometric shapes: cubes and spheres
 * Function for loading 3D model from file (in custom format)
 * Script for converting .obj file in custom format
 * Functions for rendering text on as texture or on top screen
  * Based on distance field and font atlas
 * Prototypes
  * For displaying 3D models loaded from file
  * For testing text rendering
  * For displaying generated basic geometric shapes

All this wrapped into a CMake project.

Some thoughts that behind the current code that may or may not be seen in code:
 * Keep it thin and avoid unnecessary complexity
 * Avoid allocating from heap
 * Keep structures simple and do not try to hide their sizes
 * Solve only the problems you mean to solve
 * Keep headers clean
 * Decouple as much as possible
 * Include as little as possible
 * Push information to types so that compiler can help as much as possible
 * Object orientated thinking should not be default thinking
  * Though sometimes it is a good thing

## Example
In 200 lines, show me an application that...
 * compiles on Windows and Linux
 * uses (relatively) modern OpenGL 3.2
 * logs to stdout and file
 * creates vertex and fragment shaders
 * draws a cube that can be rotated by pressing space
 * exits when pressed esc
 * updates the screen with rate of (more or less) 1/100 frames per second
 * is (relatively) clean and simple
 
See protos/demo/main.cc

## Project structure
 * TODO

## To get started
TODO

## Dependencies
TODO

## Version
Current version: too early