cmake_minimum_required(VERSION 3.20)

project(kerfengine CXX C)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(KERF_IS_TOP_LEVEL ON)
else()
    set(KERF_IS_TOP_LEVEL OFF)
endif()

option(KERF_BUILD_EXAMPLES "Build Kerfengine demo executables" ${KERF_IS_TOP_LEVEL})
option(KERF_USE_SYSTEM_GLAD "Use a host-provided GLAD instead of vendored glad.c" OFF)

if(KERF_IS_TOP_LEVEL)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    # clangd looks next to the source tree, not in the build directory.
    file(CREATE_LINK
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
    )
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)

# --------------------------------------------------
# Library sources (nested src/ files; top-level src/*.cpp are demos)
# --------------------------------------------------

file(GLOB TOP_LEVEL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(KERF_SOURCES
    src/engine/Engine.cpp
    src/engine/input/Keyboard.cpp
    src/engine/input/Mouse.cpp
    src/engine/node/Button.cpp
    src/engine/node/Drag.cpp
    src/engine/node/Drop.cpp
    src/engine/node/Model.cpp
    src/engine/node/Model3d.cpp
    src/engine/node/Node.cpp
    src/engine/node/Node2d.cpp
    src/engine/node/Node3d.cpp
    src/engine/node/RigidBody.cpp
    src/engine/node/Textbox.cpp
    src/engine/physics/Solver.cpp
    src/engine/physics/collision/Collider2D.cpp
    src/engine/physics/collision/ColliderPolygon2DMesh.cpp
    src/engine/render/buffer/FrameBuffer.cpp
    src/engine/render/pipeline/Pipeline.cpp
    src/engine/render/camera/Camera.cpp
    src/engine/render/context/Context.cpp
    src/engine/render/context/GLState.cpp
    src/engine/render/material/Material.cpp
    src/engine/resource/Font.cpp
    src/engine/resource/FontServer.cpp
    src/engine/resource/Mesh.cpp
    src/engine/resource/ObjServer.cpp
    src/engine/resource/ShaderServer.cpp
    src/engine/resource/Texture.cpp
    src/engine/resource/TextureServer.cpp
    src/engine/scene/Scene.cpp
    src/engine/text/TextLayout.cpp
)

add_library(kerf STATIC ${KERF_SOURCES})
add_library(kerf::kerf ALIAS kerf)

target_compile_features(kerf PUBLIC cxx_std_20)

# --------------------------------------------------
# GLAD: Kerfengine owns vendored GLAD unless KERF_USE_SYSTEM_GLAD=ON.
# Hosts must not compile another glad.c when linking kerf::kerf / kerf::glad.
# --------------------------------------------------
if(KERF_USE_SYSTEM_GLAD)
    if(TARGET glad::glad)
        set(KERF_GLAD_TARGET glad::glad)
    elseif(TARGET glad)
        set(KERF_GLAD_TARGET glad)
    else()
        find_package(glad CONFIG REQUIRED)
        if(TARGET glad::glad)
            set(KERF_GLAD_TARGET glad::glad)
        elseif(TARGET glad)
            set(KERF_GLAD_TARGET glad)
        else()
            message(FATAL_ERROR "KERF_USE_SYSTEM_GLAD is ON but no glad or glad::glad target was found")
        endif()
    endif()
    target_link_libraries(kerf PUBLIC ${KERF_GLAD_TARGET})
    target_compile_definitions(kerf PUBLIC GLFW_INCLUDE_NONE)
else()
    add_library(kerf_glad STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c"
    )
    add_library(kerf::glad ALIAS kerf_glad)
    target_include_directories(kerf_glad PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include"
    )
    target_compile_definitions(kerf_glad PUBLIC GLFW_INCLUDE_NONE)
    target_link_libraries(kerf PUBLIC kerf_glad)
endif()

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

target_sources(kerf PRIVATE
    "${KERF_EMBEDDED_SHADER_CPP}"
    "${KERF_EMBEDDED_MESH_CPP}"
    "${KERF_EMBEDDED_IMAGE_CPP}"
)

target_include_directories(kerf
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external"
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_compile_options(kerf PRIVATE
    -Wall -Wextra -Wpedantic
)

target_link_libraries(kerf PUBLIC
    glfw
    OpenGL::GL
    glm::glm
)

# --------------------------------------------------
# Demo executables (top-level src/*.cpp)
# --------------------------------------------------

if(KERF_BUILD_EXAMPLES)
    set(KERF_EXAMPLE_TARGETS)
    foreach(SRC_FILE ${TOP_LEVEL_SOURCES})
        get_filename_component(EXE_NAME ${SRC_FILE} NAME_WE)
        add_executable(${EXE_NAME} ${SRC_FILE})
        target_link_libraries(${EXE_NAME} PRIVATE kerf)
        add_custom_command(TARGET ${EXE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
                    "$<TARGET_FILE_DIR:${EXE_NAME}>/resources"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
                    "$<TARGET_FILE_DIR:${EXE_NAME}>/shaders"
            COMMENT "Copying resources and shaders next to ${EXE_NAME}"
        )
        list(APPEND KERF_EXAMPLE_TARGETS ${EXE_NAME})
    endforeach()

    add_custom_target(kerf_examples)
    if(KERF_EXAMPLE_TARGETS)
        add_dependencies(kerf_examples ${KERF_EXAMPLE_TARGETS})
    endif()
endif()
