#pragma once

#include <kerf/engine/resource/Texture.h>

namespace kerf {    

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

} // namespace kerf
