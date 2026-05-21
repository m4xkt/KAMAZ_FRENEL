@echo off
if not exist build mkdir build
cd build
cmake ..
cmake --build . --config Release -- -j %NUMBER_OF_PROCESSORS%
cd ..