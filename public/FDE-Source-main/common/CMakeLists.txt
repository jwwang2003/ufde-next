project(common)

# Boost_Lib
set(Boost_USE_STATIC_LIBS ON)
#set(Boost_DEBUG           ON)
find_package(Boost REQUIRED
	COMPONENTS
		date_time
		log
		program_options
		regex
)

if(FDE_ENCRYPT)
	set(EXTRA_BOOST_LIBS iostreams)
	set(EXTRA_LIBS Boost::iostreams)

	if(MSVC)
		list(APPEND EXTRA_BOOST_LIBS zlib)
		list(APPEND EXTRA_LIBS Boost::zlib)
	else()	# UNIX
		find_package(ZLIB REQUIRED)
		list(APPEND EXTRA_LIBS ZLIB::ZLIB)
	endif()

	find_package(Boost REQUIRED
		COMPONENTS ${EXTRA_BOOST_LIBS})

	file(CREATE_LINK
		${PROJECT_SOURCE_DIR}/utils/zfstream_full.h
		${PROJECT_SOURCE_DIR}/utils/zfstream.h)
else()
	file(CREATE_LINK
		${PROJECT_SOURCE_DIR}/utils/zfstream_stub.h
		${PROJECT_SOURCE_DIR}/utils/zfstream.h)
endif()

add_library(${PROJECT_NAME}
		netlist/netlist.cpp

		io/fileio.cpp
		io/loadedif.cpp
		io/nlcheck.cpp
		io/saveverilog.cpp
		io/savesimvl.cpp
		io/xml/loadxml.cpp
		io/xml/savexml.cpp
		io/xml/savertxml.cpp
		io/xml/savetxml.cpp
		io/xml/loadrtxml.cpp
		io/xml/loadtxml.cpp

		arch/archlib.cpp
		arch/loadarchlib.cpp
		rrg/rrg.cpp
		tengine/tengine.cpp

		utils/log.cpp
		report/report.cpp
)

add_library(FDE::common ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}
		${PROJECT_SOURCE_DIR}/netlist
		${PROJECT_SOURCE_DIR}/utils
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		rapidxml
		Boost::boost
)

if(MSVC)
	target_link_directories(${PROJECT_NAME}
		INTERFACE ${Boost_LIBRARY_DIRS}
	)
else()	# UNIX
	target_link_libraries(${PROJECT_NAME}
		INTERFACE
			Boost::date_time
			Boost::log
			Boost::program_options
			Boost::regex
			${EXTRA_LIBS}
	)

endif()