<p align="center">
  <img src="documentation/logo.png" alt="Picture Match logo"> 
</p>

# Picture Match
An OpenGL application like pin-the-tail-on-the-donkey -- except adapted for screens.

## Table of Contents
* [Demo Gif](#demo)
* [Key Features](#key-features)
* [Example](#example)
* [Installation](#installation)
* [Built with](#built-with)
* [Project Status](#project-status)

## Demo
<img src="https://user-images.githubusercontent.com/20725513/73604361-93abb400-455d-11ea-94fa-720686595aef.gif">

## Key Features
* Texture Mapping - Play around with vector graphics by rotating or moving 
* Texture Transformation - Rotate the eyes, ears, etc with 'J' or 'K' keys
* View Control - Resizing the window does not warp the graphics

## Installation
Picture Match uses Eigen, glad, and GLFW as submodules. To pull submodules with the project, add the argument --recurse-submodules.
`git clone --recurse-submodules https://github.com/jennarim/Picture-Match.git`

The project is compiled using CMake.
`
cd Picture-Match/
mkdir build
cd build
cmake ..
make`

Lastly, run the project with this command.
`./pictureMatch`

## Example
1. The picture of the desired image is initially shown to the player, alongside other instructions.
<img width="200" alt="1" src="https://user-images.githubusercontent.com/20725513/72637158-a5bb0f00-392e-11ea-9317-ca8660fe1dc4.png">
2. The face parts are placed and rotated randomly outside the face the user should drag the parts to.
<img width="200" alt="2" src="https://user-images.githubusercontent.com/20725513/72637258-e1ee6f80-392e-11ea-9d71-b3f4cc571f1c.png">
3. The user can rotate any of the face parts as they select it.
<img width="200" alt="3" src="https://user-images.githubusercontent.com/20725513/72637270-ec106e00-392e-11ea-84ce-c80e8c49986a.png">
4. Each face part will be bound to follow the user’s cursor.  The moment the user clicks inside the face, then the user has finalized the location.
<img width="200" alt="4" src="https://user-images.githubusercontent.com/20725513/72637287-f6cb0300-392e-11ea-8791-98ddc7b8900f.png">
5. Once all face parts have been finalized, the final result is shown.
<img width="200" alt="5" src="https://user-images.githubusercontent.com/20725513/72637307-fe8aa780-392e-11ea-8cd8-5b98e1d38ab3.png">
6. The user can press R to restart, and continue to play and admire the result.
<img width="200" alt="6" src="https://user-images.githubusercontent.com/20725513/72637323-08140f80-392f-11ea-96af-079b1895853b.png">

## Built With
* [OpenGL](https://www.opengl.org/) - the most widely adopted 2D and 3D graphics API in the industry
* [glad](https://glad.dav1d.de/) - an OpenGL loading library
* [Eigen](https://gitlab.com/libeigen/eigen) - a C++ linear algebra library
* [GLFW](https://www.glfw.org/) - an OpenGL library for creating windows and receiving input and events

## Project Status
This project is currently finished. 


