#pragma once

class Node;
class RigidBody;
class Camera;
class Button;
class Mouse;

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

    // ------------------------------------------------
    // camera
    // ------------------------------------------------

    Camera* camera = nullptr;

public:
    Scene();
    ~Scene();

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
