cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)

project ("LifeCouldBeDream")

add_subdirectory(libs/bsl)
add_subdirectory(bot)
add_subdirectory(driver)