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
        yaw(-90.0f), pitch(0.0f), movementSpeed(10.0f), drag(0.1f), gravity(-9.81f),
        isGrounded(true), model(entityModel) {
        updateOrientation();
    }

    virtual void update(float deltaTime, float groundHeight) {
        // Apply gravity if not grounded
        if (!isGrounded) {
            acceleration.y += gravity;
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

    // Render the entity
    void render(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view, const DirectionalLight sun) {
        if (model) {
            // Pass transformation data down instead of setting MVP here
            model->draw(projection, view, sun, position, glm::vec3(0.0f, -yaw, 0.0f));
        }
    }


};
