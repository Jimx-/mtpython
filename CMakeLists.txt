cmake_minimum_required(VERSION 3.10)
project(mtpython)

set(TOPDIR ${mtpython_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

include_directories(${TOPDIR}/include)

if(${UNIX})
	add_definitions(-D_UNIX_)
endif(${UNIX})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-D_LINUX_)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if(${WIN32})
	add_definitions(-D_WIN32_)
endif(${WIN32})

if(${CYGWIN})
	add_definitions(-D_CYGWIN_)
endif(${CYGWIN})

add_subdirectory(libmtpython lib)
add_subdirectory(mtpython bin)
