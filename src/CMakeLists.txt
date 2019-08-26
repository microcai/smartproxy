
include(CheckIncludeFile)

include(CheckSymbolExists)

CHECK_SYMBOL_EXISTS(MSG_DONTWAIT "sys/socket.h" HAVE_MSG_DONTWAIT)
if (HAVE_MSG_DONTWAIT)
	add_definitions(-DHAVE_MSG_DONTWAIT)
endif()

add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if (APPLE)
	set(UI ui/statusbar.h ui/statusbar.mm)
endif()

add_executable(smartproxy smartsocks5.cpp getifaddr.cpp ${UI})

target_link_libraries(smartproxy Boost::thread Boost::program_options Boost::context)

if (USE_SYSTEM_BOOST)
target_link_libraries(smartproxy Boost::coroutine)
endif()

target_link_libraries(smartproxy utility)

if (APPLE)
	target_link_libraries(smartproxy "-framework Foundation" "-framework Cocoa" objc)
endif()

install(TARGETS smartproxy DESTINATION bin)