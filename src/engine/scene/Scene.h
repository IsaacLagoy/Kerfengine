#pragma once

class Node;
class RigidBody;
class Camera;
class Button;
class Mouse;

class Scene {
private:

    // ------------------------------------------------
    // node handles
    // ------------------------------------------------

    Node* head;
    Node* tail;

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

    // TODO replace with iterators
    Node* getNodeHead() const;
    Node* getNodeTail() const;
    RigidBody* getRigidBodyHead() const;
    RigidBody* getRigidBodyTail() const;
    Button* getButtonHead() const;
    Button* getButtonTail() const;

    // ------------------------------------------------
    // rendering
    // ------------------------------------------------

    void draw() const;

    // ------------------------------------------------
    // updating
    // ------------------------------------------------
    
    void update(float dt, Mouse& mouse);
};
