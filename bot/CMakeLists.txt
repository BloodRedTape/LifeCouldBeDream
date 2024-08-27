find_package(OpenSSL REQUIRED)
find_package(tgbot REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(LifeCouldBeDreamBot
	"./sources/main.cpp"
	"./sources/simple_bot.cpp"
)

target_link_libraries(LifeCouldBeDreamBot 
	PUBLIC openssl::openssl
	PUBLIC tgbot::tgbot
	PUBLIC nlohmann_json::nlohmann_json
	PUBLIC bsl
	PUBLIC httplib 
	PUBLIC inih
)