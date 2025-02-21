cmake_minimum_required(VERSION 3.10)

project(retailnext)

set(NLOHMANN_JSON_URL "https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp")
set(NLOHMANN_JSON_FILE "json.hpp")

if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${NLOHMANN_JSON_FILE}")
    message(STATUS "Downloading json.hpp...")
    file(DOWNLOAD ${NLOHMANN_JSON_URL} "${CMAKE_CURRENT_BINARY_DIR}/${NLOHMANN_JSON_FILE}")
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_executable(retailnext main.cpp RNService.cpp)

target_include_directories(retailnext PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
