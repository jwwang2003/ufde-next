project(route)

aux_source_directory(src DIR_SRCS)

add_executable(${PROJECT_NAME} ${DIR_SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE head)

target_link_libraries(${PROJECT_NAME} PRIVATE FDE::common)

install(TARGETS ${PROJECT_NAME})