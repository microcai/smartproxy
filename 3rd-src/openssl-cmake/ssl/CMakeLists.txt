
file(GLOB SRCS *.c)

include_directories(../crypto/)


if (ANDROID)
add_library(ssl SHARED ${SRCS})
else()
add_library(ssl STATIC ${SRCS})
endif()

target_link_libraries(ssl crypto)

set_target_properties(ssl PROPERTIES FOLDER "third_party/")

target_include_directories(ssl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
