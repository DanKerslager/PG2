#pragma once
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <vector>
#include "app.hpp"

namespace Particles {

    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        float life = 1.0f;
        bool active = false;
    };

    constexpr int MAX_PARTICLES = 10000;
    inline std::vector<Particle> pool(MAX_PARTICLES);

    GLuint VAO = 0, VBO = 0;
    constexpr int MAX_DRAWABLE_PARTICLES = MAX_PARTICLES;
    bool initialized = false;

    void init() {
        if (initialized) return;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glNamedBufferData(VBO, MAX_DRAWABLE_PARTICLES * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        glBindVertexArray(0);
        initialized = true;
    }

    // Call this each frame
    inline void update(float dt) {
        for (auto& p : pool) {
            if (!p.active) continue;

            p.position += p.velocity * dt;
            p.life -= dt;

            if (p.life <= 0.0f) {
                p.active = false;
            }
        }
    }

    // Call this to spawn sparks at a position
    inline void spawn(const glm::vec3& origin, int count = 10) {
        int spawned = 0;
        for (auto& p : pool) {
            if (!p.active) {
                p.position = origin;
                p.velocity = glm::sphericalRand(5.0f); // random 3D direction
                p.life = 0.5f + static_cast<float>(rand()) / RAND_MAX; // 0.5s–1.5s
                p.active = true;
                if (++spawned >= count) break;
            }
        }
    }

    // Call this in your render loop
    void drawParticles(const glm::mat4& projection, const glm::mat4& view, ShaderProgram& shader) {
        init();

        // Collect active particle positions
        std::vector<glm::vec3> points;
        points.reserve(MAX_DRAWABLE_PARTICLES);
        for (const auto& p : pool) {
            if (p.active)
                points.push_back(p.position);
        }

        if (points.empty()) return;

        // Efficient buffer update using glMapNamedBufferRange
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glm::vec3* buffer = (glm::vec3*)glMapNamedBufferRange(VBO, 0,
            points.size() * sizeof(glm::vec3),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if (buffer) {
            memcpy(buffer, points.data(), points.size() * sizeof(glm::vec3));
            glUnmapNamedBuffer(VBO);
        }

        glBindVertexArray(VAO);
        shader.activate();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = projection * view * model;

        shader.setUniform("uMVP", mvp);
        shader.setUniform("color", glm::vec4(1, 0.7f, 0.2f, 1));
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));

        glBindVertexArray(0);
    }


} // namespace Particles
