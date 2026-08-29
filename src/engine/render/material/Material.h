#pragma once

#include "engine/resource/Texture.h"


class Material {
private:
    Texture* albedo = nullptr;
    Texture* normal = nullptr;

public:
    Material(Texture* albedo, Texture* normal = nullptr);
    ~Material();

    Texture* getAlbedo() const;
    Texture* getNormal() const;

    void setAlbedo(Texture* albedo);
    void setNormal(Texture* normal);
};
