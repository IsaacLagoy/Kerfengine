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

    // ------------------------------------------------
    // rendering
    // ------------------------------------------------
    void draw() const;

private:
    // ------------------------------------------------
    // node data structure helpers
    // ------------------------------------------------
    static void insertNode(Node* node, Node* pos);
    static void unlinkNode(Node* node);
    static void insertModel(Model* model, Model* pos);
    static void unlinkModel(Model* model);
};
