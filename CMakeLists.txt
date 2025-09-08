cmake_minimum_required(VERSION 3.10)

# Define the project name and the executable name
set(PROJECT_NAME c_markdown_blog)
set(EXEC_NAME md_web_server)
project(${PROJECT_NAME} C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Automatically find all .c source files in the src directory
file(GLOB_RECURSE SOURCE_FILES "src/*.c")

# Define the executable and its source files
add_executable(${EXEC_NAME} ${SOURCE_FILES})

# Find and link required libraries
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(${EXEC_NAME} PRIVATE cmark rt ${CMAKE_THREAD_LIibs_INIT} ${ZLIB_LIBRARIES})