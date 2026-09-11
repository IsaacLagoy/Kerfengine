#pragma once

class Node;
class Model;
class RigidBody;
class Camera;

class Scene {
private:

    // ------------------------------------------------
    // node handles
    // ------------------------------------------------

    Node* head;
    Node* tail;

    Model* modelHead;
    Model* modelTail;

    RigidBody* rigidBodyHead;
    RigidBody* rigidBodyTail;

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
    Model* getModelHead() const;
    Model* getModelTail() const;
    RigidBody* getRigidBodyHead() const;
    RigidBody* getRigidBodyTail() const;

    // ------------------------------------------------
    // rendering
    // ------------------------------------------------

    void draw() const;

    // ------------------------------------------------
    // updating
    // ------------------------------------------------
    
    void update(float dt);
};
