cmake_minimum_required(VERSION 4.0)
project(http)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)

find_package(Boost REQUIRED COMPONENTS thread)

find_package(Catch2 3 REQUIRED)
set(SOURCES src/request.cpp src/headers.cpp src/server.cpp src/response.cpp src/http.cpp)

add_library(http STATIC ${SOURCES})
target_include_directories(http 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# Build the  tests
add_executable(request_test src/request_test.cpp ${SOURCES})
target_link_libraries(request_test PRIVATE Boost::headers Boost::thread
                                           Catch2::Catch2WithMain)

# Build executable
add_executable(${PROJECT_NAME}_main)
target_sources(${PROJECT_NAME}_main PRIVATE src/main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME}_main PRIVATE Boost::headers Boost::thread)
