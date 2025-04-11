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

    constexpr int MAX_PARTICLES = 100;
    inline std::vector<Particle> pool(MAX_PARTICLES);

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
    inline void drawParticles(const glm::mat4& projection, const glm::mat4& view, ShaderProgram& shader) {
        std::vector<glm::vec3> points;
        for (const auto& p : pool) {
            if (p.active) {
                points.push_back(p.position);
            }
        }

        if (points.empty()) return;

        GLuint VAO = 0, VBO = 0;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        shader.activate();
        glm::mat4 model = glm::mat4(1.0f); // no rotation/translation
        glm::mat4 mvp = projection * view * model;

        shader.setUniform("uMVP", mvp);
        shader.setUniform("color", glm::vec4(1, 0.7f, 0.2f, 1)); // orange sparks

        glPointSize(5.0f); // optional
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));

        glBindVertexArray(0);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }



} // namespace Particles
