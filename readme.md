#### OpenCV stereovision tuner

This program let rectify stereo camera pair, create disparity map and build depth map with choosen parameters. Program was tested with two USB webcams that was joined to make stero pair.

#### Prerequirements

- Linux OS (but should be possible to run with others, but not tested)
- CMake build tool (tested: 3.2.2)
- C++ compiler with C++ 14 support (tested: gcc version 5.2.1)
- OpenCV library with Contrib module (tested 3.0.2)
- Glut library (tested freeglut 3.0.0)

#### Build

- cd stereovision
- mkdir release
- cd release
- cmake ..
- make

#### Run

Execute StereoVision in the main project directory after the build.

#### Documentation

Full usage and configuration description is available in docs directory.
