cmake_minimum_required(VERSION 3.12)
project(RaspberryPi2)
file(GLOB MY_SOURCES "./../../src/*.cpp" "./../../src/*.hpp" "./../../src/*.h")
#настройки для отладки
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(RaspberryPi2 ${MY_SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RaspberryPi2 PROPERTY CXX_STANDARD 20)
endif()

add_compile_options(-Wno-deprecated)

# Установите стандарт C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
