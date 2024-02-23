cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(telnetlibcpp)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization flag -O3 for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Include Pybind11
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.6.1  # Use the Pybind11 version that suits your needs
)
FetchContent_MakeAvailable(pybind11)

# Find required Boost components in one call
find_package(Boost REQUIRED COMPONENTS container locale)
find_package(serverpp REQUIRED)
find_package(telnetpp 3.0.0 REQUIRED)
find_package(gsl-lite REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# Source files including the Pybind11 binding file
set(SOURCE_FILES
    src/telnetlibcpp.cpp
    src/main.cpp
    src/pybind_module.cpp  # Your Pybind11 binding source file
)

# Compile as a shared library
add_library(telnetlibcpp SHARED ${SOURCE_FILES})

target_include_directories(telnetlibcpp PRIVATE include)

target_link_libraries(telnetlibcpp
    PRIVATE
        KazDragon::serverpp
        KazDragon::telnetpp
        Boost::container
        Boost::locale
        pybind11::module  # Link against Pybind11 module support
)

# Set output library name and ensure it doesn't have the 'lib' prefix
set_target_properties(telnetlibcpp PROPERTIES PREFIX "" OUTPUT_NAME "telnetlibcpp")

# Compile main.cpp as an executable
add_executable(telnetlibcpp_executable src/main.cpp)
# Make sure to include the directory for the executable as well
target_include_directories(telnetlibcpp_executable PRIVATE include)
# Link the executable with the necessary libraries
target_link_libraries(telnetlibcpp_executable
    PRIVATE
        telnetlibcpp # Link with the shared library you created
        KazDragon::serverpp
        KazDragon::telnetpp
        Boost::boost
        Boost::locale
        pybind11::embed # Link against Pybind11 embedding for executables, if needed
)