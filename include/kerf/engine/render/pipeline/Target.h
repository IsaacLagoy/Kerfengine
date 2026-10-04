#pragma once

#include <kerf/engine/render/pipeline/Format.h>
#include <kerf/engine/render/pipeline/TargetSize.h>

#include <glm/glm.hpp>

#include <initializer_list>
#include <memory>
#include <span>
#include <string>


namespace kerf {

class Shader;

enum class PresentFit {
    Stretch,
    Letterbox, // fit X to window, keep Y aspect ratio
    Letterboy, // fit Y to window, keep X aspect ratio
};

struct Plane {
    std::string name;
    Format format;
    glm::vec4 clear{0.0f, 0.0f, 0.0f, 0.0f};
    bool clearBeforeRender = true;
};

class Target {
private:
    struct Impl;
    std::shared_ptr<Impl> impl;
    
public:
    Target() = default;
    Target(TargetSize size, Filter filter, std::initializer_list<Plane> planes);
    Target(TargetSize size, Format format, Filter filter, std::string name);
    ~Target();

    Target(const Target&) = default;
    Target& operator=(const Target&) = default;
    Target(Target&&) = default;
    Target& operator=(Target&&) = default;

    explicit operator bool() const 
    {
        return static_cast<bool>(impl); 
    }

    bool operator==(const Target& other) const 
    { 
        return impl == other.impl; 
    }

    bool operator!=(const Target& other) const 
    { 
        return impl != other.impl; 
    }

    Filter filter() const;
    int width() const;
    int height() const;

    // Returns true if GPU storage changed.
    bool ensureSize(int framebufferWidth, int framebufferHeight) const;

    void beginOutput() const;
    void endOutput() const;
    void clearPlane(const std::string& name) const;

    static void bindSamplers(Shader* shader, std::span<const Target> inputs);

    void beginPresent(Shader* shader, Filter presentFilter) const;
    void endPresent() const;
};

} // namespace kerf
