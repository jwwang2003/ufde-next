#BOOST_LIB
SET(BOOST_ROOT ../../../boost_1_58_0)
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)
SET(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.58.0 REQUIRED COMPONENTS 
	atomic
	chrono
	date_time 
	iostreams 
	log_setup 
	log 	
	filesystem
	program_options
	regex 
	system 
	thread)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

#ADD -pthread
find_package(Threads REQUIRED)

#RAPIDXML_LIB
SET(Rapidxml_ROOT ../../../rapidxml)
INCLUDE_DIRECTORIES(${Rapidxml_ROOT})

INCLUDE_DIRECTORIES(../head)
INCLUDE_DIRECTORIES(../../common)
INCLUDE_DIRECTORIES(../../common/netlist)
INCLUDE_DIRECTORIES(../../common/utils)

AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_EXECUTABLE(router ${DIR_SRCS})

TARGET_LINK_LIBRARIES(router Common ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} /usr/local/lib/libz.a)
