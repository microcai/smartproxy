
include(CheckIncludeFile)

include(CheckSymbolExists)

CHECK_SYMBOL_EXISTS(MSG_DONTWAIT "sys/socket.h" HAVE_MSG_DONTWAIT)
if (HAVE_MSG_DONTWAIT)
	add_definitions(-DHAVE_MSG_DONTWAIT)
endif()

add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(smartproxy smartsocks5.cpp getifaddr.cpp)

target_link_libraries(smartproxy Boost::thread Boost::program_options Boost::context)

if (USE_SYSTEM_BOOST)
target_link_libraries(smartproxy Boost::coroutine)
endif()

target_link_libraries(smartproxy utility)


install(TARGETS smartproxy DESTINATION bin)