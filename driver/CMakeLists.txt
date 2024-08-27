

add_executable(LifeCouldBeDreamDriver
	"./sources/main.cpp"
)

target_link_libraries(LifeCouldBeDreamDriver 
	PUBLIC httplib 
	PUBLIC bsl
)