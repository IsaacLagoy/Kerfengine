#include <kerf/engine/render/material/Material.h>

namespace kerf {
    
Material::Material(Texture* albedo, Texture* normal) : albedo(albedo), normal(normal) {}

Material::~Material() {}

Texture* Material::getAlbedo() const { return albedo; }
Texture* Material::getNormal() const { return normal; }

void Material::setAlbedo(Texture* albedo) { this->albedo = albedo; }
void Material::setNormal(Texture* normal) { this->normal = normal; }

} // namespace kerf