cmake_minimum_required(VERSION 3.20)
project(RobotDance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source Files
add_executable(RobotDance main.cpp)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(RobotDance PRIVATE OpenGL::GL)

#GLUT
find_path(GLUT_INCLUDE_DIR Gl/glut.h)
find_library(GLUT_LIBRARY NAMES freegluut GLUT glut32)

if (GLUT_INCLUDE_DIR AND GLUT_LIBRARY)
	target_include_directories(RobotDance PRIVATE ${GLUT_INCLUDE_DIR})
	target_link_libraries(RobotDance PRIVATE ${GLUT_LIBRARY})
else()
	message(FATAL_ERROR "GLUT not found")
endif()

#Windows
if (WIN32)
	target_link_libraries(RobotDance PRIVATE winmm)
endif()