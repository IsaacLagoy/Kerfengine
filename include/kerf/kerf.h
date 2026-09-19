#pragma once

// GLFW must not pull in the system OpenGL headers before GLAD.
#include <glad/glad.h>

// IWYU pragma: begin_exports

// input
#include <kerf/engine/input/Keyboard.h>
#include <kerf/engine/input/Mouse.h>

// nodes
#include <kerf/engine/node/Button.h>
#include <kerf/engine/node/Drag.h>
#include <kerf/engine/node/Drop.h>
#include <kerf/engine/node/Model.h>
#include <kerf/engine/node/Model3d.h>
#include <kerf/engine/node/Node.h>
#include <kerf/engine/node/Node2d.h>
#include <kerf/engine/node/Node3d.h>
#include <kerf/engine/node/RigidBody.h>
#include <kerf/engine/node/Textbox.h>

// physics
#include <kerf/engine/physics/collision/Collider2D.h>
#include <kerf/engine/physics/collision/ColliderPolygon2DMesh.h>

// render
#include <kerf/engine/render/buffer/FrameBuffer.h>
#include <kerf/engine/render/camera/Camera.h>
// don't add context
#include <kerf/engine/render/material/Material.h>

// resources
#include <kerf/engine/resource/Font.h>
#include <kerf/engine/resource/FontServer.h>
#include <kerf/engine/resource/Mesh.h>
#include <kerf/engine/resource/ObjServer.h>
#include <kerf/engine/resource/ShaderServer.h>
#include <kerf/engine/resource/Texture.h>
#include <kerf/engine/resource/TextureServer.h>

// scene
#include <kerf/engine/scene/Scene.h>

// text
// don't add text layout

// engine
#include <kerf/engine/Engine.h>

// IWYU pragma: end_exports
