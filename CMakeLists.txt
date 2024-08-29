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
add_subdirectory(driver)

add_executable(LifeCouldBeDreamBot
	"./bot/sources/main.cpp"
	"./bot/sources/simple_bot.cpp"
	"./bot/sources/bot.cpp"
 "bot/sources/driver.cpp" "bot/sources/dream.cpp")

target_link_libraries(LifeCouldBeDreamBot 
	PUBLIC TgBot
	PUBLIC nlohmann_json
	PUBLIC bsl
	PUBLIC httplib
	PUBLIC inih
)

target_compile_definitions(LifeCouldBeDreamBot PUBLIC BSL_WITH_TIME_LOG=1)

if(CMAKE_WITH_CONAN)
	find_package(OpenSSL)
	find_package(boost)
	target_link_libraries(LifeCouldBeDreamBot PUBLIC openssl::openssl PUBLIC boost::boost)
else()
	target_link_libraries(LifeCouldBeDreamBot PUBLIC ssl PUBLIC crypto)
endif()