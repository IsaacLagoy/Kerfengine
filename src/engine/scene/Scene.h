#pragma once

class Node;
class Model;
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

    // ------------------------------------------------
    // rendering
    // ------------------------------------------------
    void draw() const;
};
