# CMake Version Requirement
cmake_minimum_required(VERSION 3.8)

project(FDE)

# user defined variables
set(FDE_ENCRYPT OFF)	# if FDE supports encrypted files
# set(BOOST_ROOT "/Users/taruhi/Project/fde/fde-master/libs")
# set(Qt5_DIR    "C:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")

# change the default install directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}" CACHE PATH "install destination" FORCE)
endif()

# Set C++11 Standard
set(CMAKE_CXX_STANDARD 17)

# gcc/clang flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-private-field -Wno-unused-local-typedefs -Wno-unused-local-typedef -Wno-unused-value -Wno-unused-result -Wno-unused-label")
endif()

add_subdirectory(3rdparty/rapidxml)
add_subdirectory(common)
add_subdirectory(vl2xml)
add_subdirectory(mapping)
add_subdirectory(packing)
add_subdirectory(placer)
add_subdirectory(router)
add_subdirectory(NLFiner)
add_subdirectory(sta)
add_subdirectory(bitgen)
# add_subdirectory(viewer)

add_definitions(SYSTEM -DBOOST_TIMER_ENABLE_DEPRECATED)
