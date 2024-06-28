find_package(openssl REQUIRED)
find_package(httplib REQUIRED)
find_package(inih REQUIRED)
find_package(tgbot REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(LifeCouldBeDreamBot
	"./sources/main.cpp"
	"./sources/simple_bot.cpp"
)

target_link_libraries(LifeCouldBeDreamBot 
	PUBLIC openssl::openssl
	PUBLIC httplib::httplib 
	PUBLIC inih::inih
	PUBLIC tgbot::tgbot
	PUBLIC nlohmann_json::nlohmann_json
	PUBLIC bsl
)