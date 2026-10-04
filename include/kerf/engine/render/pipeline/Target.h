#pragma once

#include <kerf/engine/render/pipeline/Format.h>
#include <kerf/engine/render/pipeline/TargetSize.h>

#include <glm/glm.hpp>

#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <vector>


namespace kerf {

// forward declarations
class Shader;
class Texture;
class FrameBuffer;

// ------------------------------------------------------------
// PresentFit
// ------------------------------------------------------------

enum class PresentFit {
    Stretch,
    Letterbox, // fit X to window, keep Y aspect ratio
    Letterboy, // fit Y to window, keep X aspect ratio // TODO
};

// ------------------------------------------------------------
// TargetPlane
// ------------------------------------------------------------

class TargetPlane {
private:
    std::string name;
    Format format;
    glm::vec4 clear;
    bool clearBeforeRender;

    std::unique_ptr<Texture> texture;
    int colorIndex = -1;

public:
    TargetPlane(std::string name, Format format);
    TargetPlane(std::string name, Format format, const glm::vec4& clear, bool clearBeforeRender);

    // getters
    const std::string& getName() const;
    Format getFormat() const;
    const glm::vec4& getClear() const;
    bool getClearBeforeRender() const;
    Texture* getTexture() const;
    int getColorIndex() const;

    // setters
    void setClear(const glm::vec4& clear);
    void setColorIndex(int colorIndex);
    void setTexture(std::unique_ptr<Texture> texture);
};

// ------------------------------------------------------------
// Target
// ------------------------------------------------------------

// Private implementation object for Target (pimpl / handle-body).
class TargetImpl {
public:
    TargetSize size;
    Filter filter = Filter::Linear;
    std::vector<TargetPlane> planes;
    
    int width = 0;
    int height = 0;

    std::unique_ptr<FrameBuffer> fbo;
    Texture* presentColor = nullptr;

    TargetImpl() = default;

    TargetPlane* findPlane(const std::string& name);
    Texture* soleColorTexture() const;
    void clearBound(const TargetPlane& plane) const;
    void allocate(int w, int h);
};

// ------------------------------------------------------------
// Target
// ------------------------------------------------------------

class Target {
private:
    std::shared_ptr<TargetImpl> impl;

public:
    Target() = default;
    Target(TargetSize size, Filter filter, std::initializer_list<TargetPlane> planes);
    Target(TargetSize size, Format format, Filter filter, std::string name);
    ~Target() = default;

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
