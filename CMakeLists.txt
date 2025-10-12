cmake_minimum_required(VERSION 3.16)

project(beep LANGUAGES CXX VERSION 0.01)
.git
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE "third_party/CLI11/include")