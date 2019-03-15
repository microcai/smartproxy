
include(CheckIncludeFile)

include(CheckSymbolExists)

CHECK_SYMBOL_EXISTS(MSG_DONTWAIT "sys/socket.h" HAVE_MSG_DONTWAIT)
if (HAVE_MSG_DONTWAIT)
	add_definitions(-DHAVE_MSG_DONTWAIT)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(smartsocks5 smartsocks5.cpp)

target_link_libraries(smartsocks5 Boost::thread Boost::program_options Boost::context)

target_link_libraries(smartsocks5 utility)


install(TARGETS smartsocks5 DESTINATION bin)