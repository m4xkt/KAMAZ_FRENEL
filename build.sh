#!/bin/bash

echo "=== Fresnel Diffraction Simulation Build Script ==="

BUILD_DIR="build"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Запускаем CMake. Он сам найдет OpenCV через find_package.
# Если не найдет, можно явно указать путь через -DOpenCV_DIR=/ucrt64/lib/cmake/opencv4
cmake -G "Unix Makefiles" ..

if [ $? -ne 0 ]; then
    echo "Ошибка конфигурации CMake. Проверьте вывод выше."
    exit 1
fi

make -j4

if [ $? -ne 0 ]; then
    echo "Ошибка сборки"
    exit 1
fi

echo "Сборка успешна. Запуск программы..."
./fresnel_sim