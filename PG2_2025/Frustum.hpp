#pragma once
#include <glm/glm.hpp>

struct Frustum {
    glm::vec4 planes[6]; // Left, Right, Bottom, Top, Near, Far
};

inline Frustum extractFrustum(const glm::mat4& vp) {
    Frustum f;

    // Each plane: a*x + b*y + c*z + d = 0
    // We extract planes from the combined View-Projection matrix (column-major)

    // Left  = row4 + row1
    f.planes[0] = glm::vec4(
        vp[0][3] + vp[0][0],
        vp[1][3] + vp[1][0],
        vp[2][3] + vp[2][0],
        vp[3][3] + vp[3][0]
    );
    // Right = row4 - row1
    f.planes[1] = glm::vec4(
        vp[0][3] - vp[0][0],
        vp[1][3] - vp[1][0],
        vp[2][3] - vp[2][0],
        vp[3][3] - vp[3][0]
    );
    // Bottom = row4 + row2
    f.planes[2] = glm::vec4(
        vp[0][3] + vp[0][1],
        vp[1][3] + vp[1][1],
        vp[2][3] + vp[2][1],
        vp[3][3] + vp[3][1]
    );
    // Top = row4 - row2
    f.planes[3] = glm::vec4(
        vp[0][3] - vp[0][1],
        vp[1][3] - vp[1][1],
        vp[2][3] - vp[2][1],
        vp[3][3] - vp[3][1]
    );
    // Near = row4 + row3
    f.planes[4] = glm::vec4(
        vp[0][3] + vp[0][2],
        vp[1][3] + vp[1][2],
        vp[2][3] + vp[2][2],
        vp[3][3] + vp[3][2]
    );
    // Far = row4 - row3
    f.planes[5] = glm::vec4(
        vp[0][3] - vp[0][2],
        vp[1][3] - vp[1][2],
        vp[2][3] - vp[2][2],
        vp[3][3] - vp[3][2]
    );

    // Normalize all planes (a, b, c) components
    for (auto& plane : f.planes) {
        float length = glm::length(glm::vec3(plane));
        if (length > 0.0f) {
            plane /= length;
        }
    }

    return f;
}

inline bool isInsideFrustum(const Frustum& frustum, float radius, const glm::vec3& position) {
    for (const auto& plane : frustum.planes) {
        float distance = glm::dot(glm::vec3(plane), position) + plane.w;
        if (distance < -radius) {
            return false;
        }
    }

    return true; // Intersects or inside all planes
}
