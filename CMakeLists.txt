cmake_minimum_required(VERSION 3.10)
project(raytmath)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lista de arquivos fonte
set(SOURCES
    src/core/main.cpp
    src/core/App.cpp
    src/integrators/integrator.cpp
    src/core/background.cpp
    src/core/camera.cpp
    src/core/film.cpp
    src/core/image_io.cpp
    src/core/parser.cpp
    src/core/sphere.cpp
    msg_system/error.cpp
    lib_tinyxml2/tinyxml2.cpp
)

# Cria o executável
add_executable(raytmath ${SOURCES})