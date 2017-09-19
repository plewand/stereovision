cmake_minimum_required (VERSION 2.6)
project (StereoVision)

set (StereoVision_VERSION_MAJOR 1)
set (StereoVision_VERSION_MINOR 0)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")


set ( PROJECT_LINK_LIBS -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_features2d -lopencv_calib3d -lopencv_imgcodecs -lopencv_videoio -lopencv_photo -lstdc++fs -lglut -lGL -lGLU -lopencv_ximgproc)

add_executable(StereoVision  ${SOURCES})
target_link_libraries(StereoVision ${PROJECT_LINK_LIBS} )

