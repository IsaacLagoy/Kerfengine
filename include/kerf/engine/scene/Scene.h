#pragma once

namespace kerf {

// forward declarations
class Node;
class RigidBody;
class Camera;
class Button;
class Mouse;
class Drag;

// ------------------------------------------------
// Scene
// ------------------------------------------------

class Scene {
    friend class Node;
    friend class RigidBody;
    friend class Button;

private:

    // ------------------------------------------------
    // node handles
    // ------------------------------------------------

    Node* root;

    RigidBody* rigidBodyHead;
    RigidBody* rigidBodyTail;

    Button* buttonHead;
    Button* buttonTail;

    Drag* selectedDrag = nullptr;

    // ------------------------------------------------
    // camera
    // ------------------------------------------------

    Camera* camera = nullptr;

public:
    Scene();
    ~Scene();

    // ------------------------------------------------
    // camera
    // ------------------------------------------------

    void setCamera(Camera* camera);
    Camera* getCamera() const;

    // ------------------------------------------------
    // node management
    // ------------------------------------------------

    void addNode(Node* node);
    void removeNode(Node* node);

    // testing functions
    int getRigidBodyCount() const;
    int getButtonCount() const;

    void setSelectedDrag(Drag* drag);
    Drag* getSelectedDrag() const;

    // ------------------------------------------------
    // rendering
    // ------------------------------------------------

    void draw() const;

    // ------------------------------------------------
    // updating
    // ------------------------------------------------
    
    void update(float dt, Mouse& mouse);

private:
    Node* getRoot() const;
    RigidBody* getRigidBodyHead() const;
    RigidBody* getRigidBodyTail() const;
    Button* getButtonHead() const;
    Button* getButtonTail() const;
};

} // namespace kerf