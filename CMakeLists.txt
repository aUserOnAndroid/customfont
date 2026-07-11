cmake_minimum_required(VERSION 3.10)
project(customfont)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Freetype REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/font_loader.cpp
)

set(HEADERS
    include/font_loader.h
)

# Create library
add_library(customfont ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(customfont
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(customfont
    PUBLIC
        ${FREETYPE_LIBRARIES}
)

# Create executable (optional)
add_executable(customfont_demo src/main.cpp)
target_link_libraries(customfont_demo customfont)
