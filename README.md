# Kerfengine

Requires CMake 3.20+, C++20, OpenGL, [GLFW](https://www.glfw.org/) (`glfw3` CMake package), and [GLM](https://github.com/g-truc/glm). Kerfengine is MIT licensed; see `LICENSE`.

## Usage

Add as a git submodule (or otherwise place the tree under your project) and link `kerf::kerf`. Examples stay off unless you turn them on.

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_game CXX)

add_subdirectory(third_party/Kerfengine)  # KERF_BUILD_EXAMPLES defaults OFF
add_executable(my_game main.cpp)
target_link_libraries(my_game PRIVATE kerf::kerf)
```

Kerfengine's CMake calls `find_package(OpenGL)`, `find_package(glfw3 CONFIG)`, and `find_package(glm)` itself. Make those packages visible the same way you would for any other CMake project (Homebrew, vcpkg, `CMAKE_PREFIX_PATH`, and so on).

In C++:

```cpp
#include <kerf/kerf.h>
```

Public headers include `<glad/glad.h>`. Linking `kerf::kerf` defines `GLFW_INCLUDE_NONE` so GLFW does not pull system OpenGL headers before GLAD. Do not compile another `glad.c` in the host; that duplicates `glad*` symbols. If the host already has GLAD, configure Kerfengine with `-DKERF_USE_SYSTEM_GLAD=ON` and provide a `glad` or `glad::glad` CMake target before `add_subdirectory`.

Default shaders, `unit.obj`, and `white.png` are embedded in the library. Extra demo assets on disk are only for the example programs.

## Developer

Configure and build from the repo root. Out-of-source `build/` is the usual layout:

```bash
cmake -S . -B build
cmake --build build
```

That builds the `kerf` library, vendored `kerf_glad`, and every demo under `src/*.cpp`. After each demo links, `resources/` and `shaders/` are copied next to that binary. clangd gets a `compile_commands.json` symlink next to this source tree (top-level configures only).

Useful targets and options:

```bash
cmake --build build --target kerf            # library only
cmake --build build --target kerf_examples   # all demos
cmake --build build --target kerf_glad       # vendored GLAD

cmake -S . -B build -DKERF_BUILD_EXAMPLES=OFF
```

`KERF_BUILD_EXAMPLES` defaults **ON** when this repo is the CMake source root and **OFF** when it is `add_subdirectory`. The compile-commands symlink is top-level only; example asset copies run whenever examples are built.

```bash
find ../src ../include -type f -exec wc -l {} +
```

Notes about node transfer between scenes and between nodes

- Node trees must all have the same scene. If possible, remove the scene back reference entirely
- When a node is reparented or orphaned, it should keep all of its children.
- If a node is removed from a scene, it should not be deleted and take all of its children with it
- When a node is added to a scene, all of its children should be added to the respective linked lists. When it is removed, all of the children should be removed from the linked lists.
- Nodes should be able to exist without a scene in a hierarchy as normal
- For now, copy and move operators are disabled to make implementation easier

