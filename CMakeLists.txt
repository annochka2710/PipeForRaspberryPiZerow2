cmake_minimum_required(VERSION 3.12)
project(RaspberryPi2)

# Настройки для отладки
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Установите стандарт C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Убираем проблемный флаг для MSVC, добавляем только для GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated)
elseif(MSVC)
    # Для MSVC используем эквивалентные флаги
    add_compile_options(/wd4996)  # Отключает предупреждения о deprecated функциях
endif()

# Копируем ресурсы в папку сборки ← ДОБАВЬТЕ ЭТУ СТРОКУ
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Собираем список исходных файлов - правильные пути
file(GLOB MY_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp" 
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Создаем исполняемый файл
add_executable(RaspberryPi2 ${MY_SOURCES})

# Для Windows может потребоваться
if(WIN32)
    target_link_libraries(RaspberryPi2 ws2_32)
endif()

# Для Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(RaspberryPi2 pthread)
endif()

# Для работы с файловой системой C++17
#target_link_libraries(RaspberryPi2 stdc++fs)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()