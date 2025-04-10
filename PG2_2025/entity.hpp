#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.hpp"
#include "LightSource.hpp"

class Entity {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    using Behavior = std::function<void(Entity&, float)>;
    std::vector<Behavior> behaviors;

    float yaw;
    float pitch;

    float movementSpeed;
    float drag;
    float gravity;
    bool isGrounded;

    Model* model; // Pointer to a 3D model (optional)

    Entity(glm::vec3 startPosition, Model* entityModel = nullptr)
        : position(startPosition), velocity(glm::vec3(0.0f)), acceleration(glm::vec3(0.0f)),
        front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), worldUp(up),
        yaw(-90.0f), pitch(0.0f), movementSpeed(100.0f), drag(0.1f), gravity(-9.81f),
        isGrounded(true), model(entityModel) {
        updateOrientation();
    }

    virtual void update(float deltaTime, float groundHeight) {
        // Apply gravity if not grounded
        if (!isGrounded) {
            acceleration.y += gravity;
        }

        for (auto& b : behaviors) {
            b(*this, deltaTime);
        }

        // Integrate acceleration into velocity
        velocity += acceleration * deltaTime;

        // Apply drag only to horizontal movement
        velocity.x *= pow(drag, deltaTime);
        velocity.z *= pow(drag, deltaTime);

        // Update position
        position += velocity * deltaTime;

        // Check for ground collision
        if (position.y <= groundHeight) {
            position.y = groundHeight;
            velocity.y = 0;
            isGrounded = true;
        }
        else {
            isGrounded = false;
        }

        // Reset acceleration (forces apply for one frame)
        acceleration = glm::vec3(0.0f);
    }

    void addBehavior(Behavior b) {
        behaviors.push_back(std::move(b));
    }

    void applyForce(glm::vec3 force) {
        acceleration += force;
    }

    virtual void jump(float jumpStrength) {
        if (isGrounded) {
            velocity.y = jumpStrength;
            isGrounded = false;
        }
    }

    // Rotate the entity using yaw & pitch
    void rotate(float deltaYaw, float deltaPitch) {
        yaw += deltaYaw;
        pitch += deltaPitch;

        // Clamp pitch to avoid flipping
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateOrientation();
    }

    // Update front, right, up vectors based on yaw & pitch
    void updateOrientation() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

    float getCollisionRadius() const {
        return glm::length(model->boundingBoxMax - model->boundingBoxMin) * 0.5f;
    }

    // Render the entity
    void render(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view, const std::vector<LightSource*> lights) {
        if (model) {
            // Pass transformation data down instead of setting MVP here
            model->draw(projection, view, lights, position, glm::vec3(0.0f, -yaw, 0.0f));
        }
    }

    void drawBoundingBox(const glm::mat4& projection, const glm::mat4& view, ShaderProgram& debugShader) const {
        if (!model) {
            std::cerr << "[drawBoundingBox] Warning: Entity has no model.\n";
            return;
        }

        if (model->boundingBoxMin == model->boundingBoxMax) {
            std::cerr << "[drawBoundingBox] Warning: Bounding box is degenerate (likely uninitialized).\n";
            return;
        }

        glm::vec3 min = model->boundingBoxMin;
        glm::vec3 max = model->boundingBoxMax;

        glm::vec3 corners[8] = {
            {min.x, min.y, min.z}, {max.x, min.y, min.z},
            {max.x, max.y, min.z}, {min.x, max.y, min.z},
            {min.x, min.y, max.z}, {max.x, min.y, max.z},
            {max.x, max.y, max.z}, {min.x, max.y, max.z}
        };

        int indices[24] = {
            0,1, 1,2, 2,3, 3,0,
            4,5, 5,6, 6,7, 7,4,
            0,4, 1,5, 2,6, 3,7
        };

        std::vector<glm::vec3> lines;
        lines.reserve(24);
        for (int i = 0; i < 24; ++i)
            lines.push_back(corners[indices[i]]);

        GLuint VAO = 0, VBO = 0;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

        debugShader.activate();
        // Only rotate around Y (yaw), ignore pitch for flat rotation
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(-yaw), glm::vec3(0, 1, 0));

        glm::mat4 mvp = projection * view * model;

        debugShader.setUniform("uMVP", mvp);
        debugShader.setUniform("color", glm::vec4(1, 0, 0, 1)); // Red

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()));
        glBindVertexArray(0);

        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

};
