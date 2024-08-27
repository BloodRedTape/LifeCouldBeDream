

add_subdirectory(../libs/bsl bsl)

add_executable(LifeCouldBeDreamDriver
	"./sources/main.cpp"
)

target_link_libraries(LifeCouldBeDreamDriver 
	PUBLIC cpp-httplib 
	PUBLIC bsl
)