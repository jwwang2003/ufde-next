project(import)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(VlScanner vl2xml_lexer.lxx  ${CMAKE_CURRENT_BINARY_DIR}/vl2xml_lexer.cpp)
BISON_TARGET(VlParser vl2xml_parser.ypp ${CMAKE_CURRENT_BINARY_DIR}/vl2xml_parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(VlScanner VlParser)

add_executable(${PROJECT_NAME} 
	main.cpp
	module.cpp
	DebugHelper.cpp
	ParseDesign.cpp
	VerilogLoader.cpp
	VerilogMisc.cpp
	Vl2xmlApp.cpp
	${FLEX_VlScanner_OUTPUTS}
	${BISON_VlParser_OUTPUTS}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE . ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME}
	PRIVATE FDE::common) #${FLEX_LIBRARIES})

install(TARGETS ${PROJECT_NAME})