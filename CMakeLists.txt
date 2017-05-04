cmake_minimum_required(VERSION 2.8)

# guard against in-source builds
#if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
#    message(FATAL_ERROR "In-source builds not allowed.")
#endif()

set(SOURCE_DIRS
    src
    src/ext_sort
    src/utils
    src/utils/fs
    src/utils/log
    src/utils/log/loggers
)

set(PROJECT_NAME ExternalSort)
project(${PROJECT_NAME})
include_directories(
    /usr/local/include
    src
)

link_directories(
    /usr/local/lib
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/${CMAKE_BUILD_TYPE})

foreach(DIR ${SOURCE_DIRS})
    file(GLOB SOURCES_CPP ${DIR}/*.cpp)
    file(GLOB SOURCES_H ${DIR}/*.h)
    source_group(${DIR} FILES ${SOURCES_CPP} ${SOURCES_H})
    set(SOURCES ${SOURCES} ${SOURCES_CPP} ${SOURCES_H})
endforeach()

add_executable(${PROJECT_NAME} ${SOURCES})
if (NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread)
endif()

if (WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -D_FILE_OFFSET_BITS=64")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic-errors -Wall -Waddress -Wextra -D_FILE_OFFSET_BITS=64")
endif()

#install(TARGETS ${PROJECT_NAME} DESTINATION ".")
