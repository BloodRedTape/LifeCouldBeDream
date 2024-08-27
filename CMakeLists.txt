cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)

project ("LifeCouldBeDream")


add_library(inih STATIC "libs/inih/ini.c" "libs/inih/cpp/INIReader.cpp")
target_include_directories(inih PUBLIC "libs/inih" PUBLIC "libs/inih/cpp")

add_subdirectory(libs/bsl)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF)
add_subdirectory(libs/cpp-httplib)
add_subdirectory(libs/tgbot-cpp)
add_subdirectory(libs/json)
add_subdirectory(bot)