

find_package(openssl REQUIRED)
find_package(httplib REQUIRED)
find_package(inih REQUIRED)

add_executable(LifeCouldBeDreamDriver
	"./sources/main.cpp"
)

target_link_libraries(LifeCouldBeDreamDriver 
	PUBLIC openssl::openssl
	PUBLIC httplib::httplib 
	PUBLIC inih::inih
	PUBLIC bsl
)