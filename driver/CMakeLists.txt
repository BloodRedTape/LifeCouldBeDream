

add_subdirectory(../libs/bsl bsl)

add_executable(LifeCouldBeDreamDriver
	"./sources/main.cpp"
)

target_compile_definitions(LifeCouldBeDreamDriver PUBLIC BSL_WITH_TIME_LOG=1)

target_link_libraries(LifeCouldBeDreamDriver 
	PUBLIC cpp-httplib 
	PUBLIC bsl
)