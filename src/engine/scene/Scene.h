#pragma once

class Node;
class Model;
class Camera;
class RigidBody;

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

    Camera* camera = nullptr;

public:
    Scene();
    ~Scene();

    void addNode(Node* node);
    void removeNode(Node* node);

    void setCamera(Camera* camera);
    Camera* getCamera() const;

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
