project(map)

add_executable(${PROJECT_NAME}
	main.cpp
	args.cpp
	mapping.cpp
	patternmap.cpp
	aigtransform.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE FDE::common)

install(TARGETS ${PROJECT_NAME})