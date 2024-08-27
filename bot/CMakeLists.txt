add_executable(LifeCouldBeDreamBot
	"./sources/main.cpp"
	"./sources/simple_bot.cpp"
)

target_link_libraries(LifeCouldBeDreamBot 
	PUBLIC tgbot
	PUBLIC nlohmann_json
	PUBLIC bsl
	PUBLIC httplib 
	PUBLIC inih
)

if(CMAKE_WITH_CONAN)
	find_package(OpenSSL)
	target_link_libraries(LifeCouldBeDreamBot PUBLIC openssl::openssl)
else()
	target_link_libraries(LifeCouldBeDreamBot PUBLIC ssl PUBLIC crypto)
endif()