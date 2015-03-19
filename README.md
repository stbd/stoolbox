# SToolbox
Storytelling Toolbox: a small collection of tools to get started with visual applications.

## What is this?
Well, first of all this is a hobby project (so be gentle). The idea of SToolbox is to be some kind of collection of tools that help you get started with visual applications on top of OpenGL, as well as a set of scripts that prepare a development environment by installing commonly used libraries. And, since it is a hobby project, it is a place to test and learn about software architectures and development.

Currently what this includes is scripts that compile and install libraries that include SDL2, Boost, GLM, GLLoadgen. And APIs for creating OpenGL shaders, Vertex Array Objects, loading 3D models, and methods for generating some objects procedurally.

All this is wrapped into a CMake project.

Some thoughts that may be seen in API code:
 * Keep it thin and avoid unnecessary complexity
 * Avoid allocating from heap
 * Keep structures simple and do not try to hide their sizes
 * Solve only the problems you mean to solve
 * Keep headers clean
 * Decouple as much as possible
 * Include as little as possible
 * Push information to types so that compiler can help as much as possible
 * Object orientated thinking should not be the default
 * Though sometimes object orientated is good thinking

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
TODO

## What's included
Target: demo. An example of how to use shaders and models
Target: viewer. Prototype that uses generator functions
Scripts to setup environment for Linux (Bash scripts) and Windows (Powershell scripts). Located under libs, they include script for Boost 1.57, SDL2, GLM, GLLoadgen.

## To get started
TODO

## Dependencies
TODO
