cmake_minimum_required(VERSION 3.10)
project(customfont)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Freetype REQUIRED)

# Add source files
set(SOURCES
    src/font_loader.cpp
)

set(DEMO_SOURCES
    src/main.cpp
)

set(HEADERS
    include/font_loader.h
)

# Create shared library for Geode plugin
add_library(customfont SHARED ${SOURCES} ${HEADERS})
set_target_properties(customfont PROPERTIES OUTPUT_NAME "auseronandroid.customfont")
set_target_properties(customfont PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Include directories
target_include_directories(customfont
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(customfont
    PRIVATE
        ${FREETYPE_LIBRARIES}
)

# Create executable demo
add_executable(customfont_demo ${DEMO_SOURCES})
target_include_directories(customfont_demo PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(customfont_demo PRIVATE customfont)
