cmake_minimum_required(VERSION 3.20)

project(kerengine CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# clangd looks next to the source tree, not in the build directory.
file(CREATE_LINK
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
    SYMBOLIC
)

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

# --------------------------------------------------
# Embed shaders, meshes, and images
# --------------------------------------------------
# Shaders
file(GLOB KERF_SHADER_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)
set(KERF_EMBEDDED_SHADER_H "${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedShaders.h")
set(KERF_EMBEDDED_SHADER_CPP "${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedShaders.cpp")
add_custom_command(
    OUTPUT "${KERF_EMBEDDED_SHADER_H}" "${KERF_EMBEDDED_SHADER_CPP}"
    COMMAND ${CMAKE_COMMAND}
        -DINPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/shaders
        -DEXTENSIONS=vert,frag
        -DOUT_H=${KERF_EMBEDDED_SHADER_H}
        -DOUT_CPP=${KERF_EMBEDDED_SHADER_CPP}
        -DHEADER_NAME=EmbeddedShaders.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedText.cmake
    DEPENDS ${KERF_SHADER_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedText.cmake
    COMMENT "Embedding GLSL shaders"
    VERBATIM
)

# Meshes
file(GLOB KERF_MESH_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/mesh/*.obj"
)
set(KERF_EMBEDDED_MESH_H "${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedMeshes.h")
set(KERF_EMBEDDED_MESH_CPP "${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedMeshes.cpp")
add_custom_command(
    OUTPUT "${KERF_EMBEDDED_MESH_H}" "${KERF_EMBEDDED_MESH_CPP}"
    COMMAND ${CMAKE_COMMAND}
        -DINPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/resources/mesh
        -DEXTENSIONS=obj
        -DOUT_H=${KERF_EMBEDDED_MESH_H}
        -DOUT_CPP=${KERF_EMBEDDED_MESH_CPP}
        -DHEADER_NAME=EmbeddedMeshes.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedText.cmake
    DEPENDS ${KERF_MESH_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedText.cmake
    COMMENT "Embedding OBJ meshes"
    VERBATIM
)

# Images
set(KERF_WHITE_PNG "${CMAKE_CURRENT_SOURCE_DIR}/resources/image/white.png")
set(KERF_EMBEDDED_IMAGE_H "${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedImages.h")
set(KERF_EMBEDDED_IMAGE_CPP "${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedImages.cpp")
add_custom_command(
    OUTPUT "${KERF_EMBEDDED_IMAGE_H}" "${KERF_EMBEDDED_IMAGE_CPP}"
    COMMAND ${CMAKE_COMMAND}
        -DINPUT_FILE=${KERF_WHITE_PNG}
        -DOUT_H=${KERF_EMBEDDED_IMAGE_H}
        -DOUT_CPP=${KERF_EMBEDDED_IMAGE_CPP}
        -DHEADER_NAME=EmbeddedImages.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedBinary.cmake
    DEPENDS ${KERF_WHITE_PNG} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedBinary.cmake
    COMMENT "Embedding white.png"
    VERBATIM
)

# --------------------------------------------------
# Link common library
# --------------------------------------------------
target_sources(common PRIVATE
    "${KERF_EMBEDDED_SHADER_CPP}"
    "${KERF_EMBEDDED_MESH_CPP}"
    "${KERF_EMBEDDED_IMAGE_CPP}"
)
target_include_directories(common PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_compile_options(common PRIVATE
    -Wall -Wextra -Wpedantic
)

target_compile_definitions(common PUBLIC
    GLFW_INCLUDE_NONE
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
