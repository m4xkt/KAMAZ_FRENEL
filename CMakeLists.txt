cmake_minimum_required(VERSION 3.10)
project(FresnelDiffraction)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

add_executable(fresnel_sim
    src/main.cpp
    src/fresnel.cpp
    src/DifrCalc.cpp
    src/ImageRender.cpp
    src/FresnelSimulation.cpp
)

target_link_libraries(fresnel_sim ${OpenCV_LIBS})
target_include_directories(fresnel_sim PRIVATE include)

