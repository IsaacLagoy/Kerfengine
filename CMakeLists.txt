cmake_minimum_required(VERSION 3.20)

project(kerengine CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)

# --------------------------------------------------
# Common library: nested src/ files + glad
# --------------------------------------------------

file(GLOB TOP_LEVEL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(REMOVE_ITEM COMMON_SOURCES ${TOP_LEVEL_SOURCES})

add_library(common STATIC
    external/glad/src/glad.c
    ${COMMON_SOURCES}
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_compile_options(common PRIVATE
    -Wall -Wextra -Wpedantic
)

target_link_libraries(common PUBLIC
    glfw
    OpenGL::GL
    glm::glm
)

# --------------------------------------------------
# One executable per src/*.cpp
# --------------------------------------------------

foreach(SRC_FILE ${TOP_LEVEL_SOURCES})
    get_filename_component(EXE_NAME ${SRC_FILE} NAME_WE)

    add_executable(${EXE_NAME} ${SRC_FILE})
    target_link_libraries(${EXE_NAME} PRIVATE common)
endforeach()

# --------------------------------------------------
# Copy resources/ -> build/resources/
# --------------------------------------------------

add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders
            ${CMAKE_BINARY_DIR}/shaders
    COMMENT "Copying resources and shaders to build directory"
)
