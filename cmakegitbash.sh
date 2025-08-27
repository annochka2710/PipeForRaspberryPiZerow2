#!/bin/bash
echo -en "\033[37;1;41m Checking build dir \033[0m"
sleep 1
if [ -d "build" ]; then
    echo "✓ Dir exists"
    rm -rf build
    mkdir build && cd build
else 
    echo " ✗ No dir"
    if mkdir build; then
        echo "✓ Dir created"
    if cd build; then
        echo "✓ In dir"
    else
        echo "Failed to create dir"
        exit 1
    fi
    else
        echo "Failed to create dir"
        exit 1
    fi
fi

#configuring
echo -en "\033[37;1;41m Configuring... \033[0m"
sleep 1
#cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake -B build -G "MinGW Makefiles"
cmake .. -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Debug

# 1. Проверка конфигурации
#cmake -B . -S .
# 2. Сборка с оптимизацией
echo -en "\033[37;1;41m Building... \033[0m"
sleep 1
cmake --build . --config Debug --parallel 4

touch pipe.bin

# 3. Запуск программы
echo -en "\033[37;1;41m Ready to exec in powershell... \033[0m"
#cd build
#.\Debug\RaspberryPi2.exe
