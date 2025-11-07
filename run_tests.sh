#!/bin/bash

echo "создание директории build..."
rm -rf build
mkdir build
cd build || exit

echo "запускаем CMake..."
cmake ..

echo "Компиляция проекта..."
make

echo "Запуск тестов CTest..."
ctest -V

echo "Тестирование завершено!"
