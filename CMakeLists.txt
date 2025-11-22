cmake_minimum_required(VERSION 3.16)

project(beep LANGUAGES CXX VERSION 0.01)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

FILE(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "third_party/CLI11/include")