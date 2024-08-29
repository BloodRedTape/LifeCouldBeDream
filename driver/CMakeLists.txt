

if(!CMAKE_WITH_CONAN)
	add_subdirectory(../libs/bsl bsl)
endif()

add_executable(LifeCouldBeDreamDriver
	"./sources/main.cpp"
)

target_compile_definitions(LifeCouldBeDreamDriver PUBLIC BSL_WITH_TIME_LOG=1)

target_link_libraries(LifeCouldBeDreamDriver 
	PUBLIC bsl
)

if(CMAKE_WITH_CONAN)
	find_package(boost)
	target_link_libraries(LifeCouldBeDreamDriver PUBLIC boost::boost)
else()
	target_link_libraries(LifeCouldBeDreamDriver PUBLIC boost)
endif()