cmake_minimum_required(VERSION 4.0)
project(http)
set(version 1.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)

find_package(Boost REQUIRED COMPONENTS thread system asio)
find_package(OpenSSL REQUIRED)

find_package(Catch2 3 REQUIRED)
set(SOURCES src/request.cpp src/headers.cpp src/server.cpp src/response.cpp
            src/http.cpp src/html.cpp)

set(HEADERS include/request.h include/headers.h include/server.h
            include/response.h include/http.h include/html.h)

add_library(http STATIC ${SOURCES})
target_include_directories(
  http PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>)
install(DIRECTORY include/ DESTINATION include)

# Install the library target and export it
install(
  TARGETS http
  EXPORT httpTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES
  DESTINATION include)

# Export the targets for use by other projects
install(
  EXPORT httpTargets
  FILE httpTargets.cmake
  NAMESPACE http::
  DESTINATION lib/cmake/http)

# Generate a Config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/httpConfigVersion.cmake"
  VERSION 1.0
  COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/httpConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/httpConfig.cmake"
  INSTALL_DESTINATION lib/cmake/http)

target_link_libraries(http PRIVATE Boost::headers Boost::thread
                                   ${OPENSSL_LIBRARIES})
# Install the Config files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/httpConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/httpConfigVersion.cmake"
        DESTINATION lib/cmake/http)

# target_include_directories(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src) Build
# the  tests
add_executable(request_test src/request_test.cpp ${SOURCES})
target_link_libraries(
  request_test PRIVATE Boost::headers Boost::thread
                       Catch2::Catch2WithMain ${OPENSSL_LIBRARIES})

# Build executable
add_executable(${PROJECT_NAME}_main)
target_sources(${PROJECT_NAME}_main PRIVATE src/main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME}_main PRIVATE Boost::headers Boost::thread ${OPENSSL_LIBRARIES})

# Copy assets folder to build dir
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets"
          "${CMAKE_CURRENT_BINARY_DIR}/assets")
