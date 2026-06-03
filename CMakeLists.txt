cmake_minimum_required(VERSION 3.16)

# Wymuszenie standardu C++26 dla nowoczesnych szablonów i smart pointerów
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_multiapp_framework)
