#pragma once

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


inline float cross(const glm::vec2& a, const glm::vec2& b)
{
    return a.x * b.y - a.y * b.x;
}

inline float cross(const glm::vec2& o, const glm::vec2& a, const glm::vec2& b)
{
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

inline bool monotoneChain(std::vector<glm::vec2>& hull, const std::vector<glm::vec2>& points)
{
    std::vector<glm::vec2> pts = points;
    hull.clear();

    // remove duplicates, sort by x then y
    std::sort(pts.begin(), pts.end(), [](const glm::vec2& a, const glm::vec2& b) {
        if (glm::abs(a.x - b.x) > glm::epsilon<float>()) return a.x < b.x;
        return a.y < b.y;
    });
    pts.erase(std::unique(pts.begin(), pts.end(),
    [](const glm::vec2& a, const glm::vec2& b) {
        return glm::distance(a, b) < 1e-9f;
    }), pts.end());

    // degenerate: 0, 1, or 2 points
    size_t n = pts.size();
    if (n <= 2) {
        hull = pts;
        return false;
    }

    std::vector<glm::vec2> lower, upper;

    // lower hull
    for (const auto& p : pts) {
        while (lower.size() >= 2 &&
               cross(lower[lower.size()-2], lower[lower.size()-1], p) <= glm::epsilon<float>()) {
            lower.pop_back();
        }
        lower.push_back(p);
    }

    // upper hull
    for (auto it = pts.rbegin(); it != pts.rend(); ++it) {
        const auto& p = *it;
        while (upper.size() >= 2 &&
               cross(upper[upper.size()-2], upper[upper.size()-1], p) <= glm::epsilon<float>()) {
            upper.pop_back();
        }
        upper.push_back(p);
    }

    // remove duplicates and close the hull
    lower.pop_back();
    upper.pop_back();
    hull = lower;
    hull.insert(hull.end(), upper.begin(), upper.end());

    // success
    return true;
}

inline float polygonArea(const std::vector<glm::vec2>& vertices)
{
    const size_t n = vertices.size();
    if (n < 3) {
        return 0.0f;
    }

    float signedArea = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        signedArea += cross(vertices[i], vertices[(i + 1) % n]);
    }
    return 0.5f * glm::abs(signedArea);
}

inline float polygonInertia(const std::vector<glm::vec2>& vertices, float density, const glm::vec2& scale = glm::vec2(1.0f))
{
    const size_t n = vertices.size();
    if (n < 3) {
        return 0.0f;
    }

    float signedArea2 = 0.0f;
    glm::vec2 centroid(0.0f);
    float inertiaOrigin = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const glm::vec2 a(vertices[i].x * scale.x, vertices[i].y * scale.y);
        const glm::vec2 b(vertices[(i + 1) % n].x * scale.x, vertices[(i + 1) % n].y * scale.y);
        const float c = cross(a, b);
        signedArea2 += c;
        centroid += (a + b) * c;
        inertiaOrigin += c * (glm::dot(a, a) + glm::dot(a, b) + glm::dot(b, b));
    }

    if (glm::abs(signedArea2) < glm::epsilon<float>()) {
        return 0.0f;
    }

    const float area = 0.5f * signedArea2;
    centroid /= (3.0f * signedArea2);
    inertiaOrigin *= density / 12.0f;

    return glm::abs(inertiaOrigin - density * area * glm::dot(centroid, centroid));
}