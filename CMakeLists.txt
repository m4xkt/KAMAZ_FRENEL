cmake_minimum_required(VERSION 3.10)
project(FresnelDiffraction)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(fresnel_sim
    main.cpp
    fresnel.cpp
    FresnelSimulation.cpp
)

target_link_libraries(fresnel_sim ${OpenCV_LIBS})