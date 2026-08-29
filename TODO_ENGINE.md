# 2D Engine TODO

What is left for a basic 2D engine. Text-system work lives in `TODO.md`.

## Structure

- [ ] **Engine loop** — move window, viewport, camera resize, clear, `scene.draw()`, swap/poll out of `main.cpp` into `Engine`. `main` should only set up the demo scene.
- [ ] **Scene ownership** — keep `Scene` as the owner of nodes; `main` should not leak models if they are never added.

## Rendering

- [ ] **Sprites** — load images into `Texture`, bind on `Model`, sample UVs in `default2d` (tint with `uColor`). Texture/FBO classes exist but are unused on this path.
- [ ] **Material** — `Model` currently hardcodes `"default2d"`. Group shader, texture, and color so a model can switch look without `Scene` knowing the shader name.

## Scene graph

- [ ] **Parent/child nodes** — pose is world-space only. Children should follow a parent transform (`Node.h` TODO).
- [ ] **Text as nodes** — fold `text.cpp` into the same `Scene::draw()` path instead of a second executable.

## Input

- [ ] **Input** — keyboard/mouse on the window. Camera already has `screenToWorld` / `worldToScreen` for pan, zoom, and picking.

## Later (not required for basic 2D)

- Batching / atlas
- `NodeType::LIGHT`
- SDF text, wrapping, Unicode (see `TODO.md`)
