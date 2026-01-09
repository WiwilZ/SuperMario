cmake_minimum_required(VERSION 3.11)
project(SuperMario LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.2
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

FetchContent_MakeAvailable(sfml)

# Collect source files
file(GLOB SOURCE_FILES "Source/*.cpp")

# Create executable
add_executable(SuperMario Source/Main.cpp)

# Include directories
target_include_directories(SuperMario PRIVATE "Source/Headers")
target_include_directories(SuperMario PRIVATE ${sfml_SOURCE_DIR}/include)

# Link SFML libraries
target_link_libraries(SuperMario sfml-graphics sfml-window sfml-system)

# Copy resources to build directory (optional)
file(COPY "Source/Resources" DESTINATION ${CMAKE_BINARY_DIR})