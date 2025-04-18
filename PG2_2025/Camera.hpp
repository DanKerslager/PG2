#pragma once
#include "Entity.hpp"

class Camera : public Entity {
public:
    float sensitivity;
    bool firstMouse;
    float lastX, lastY;
    glm::vec3 viewpointOffset; // First-person offset
    glm::vec3 thirdPersonOffset; // Offset behind the model
    bool thirdPerson; // Toggles view mode

    float camYaw = -90.0f;   // Horizontal look angle
    float camPitch = 0.0f;   // Vertical look angle
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  // Initial look direction

    Camera(glm::vec3 startPosition, Model* entityModel = nullptr)
        : Entity(startPosition, entityModel), sensitivity(0.1f), firstMouse(true),
        lastX(400), lastY(300),
        viewpointOffset(glm::vec3(0.0f, 3.0f, 0.0f)), // First-person offset (head position)
        thirdPersonOffset(glm::vec3(0.0f, 5.0f, 15.0f)), // Third-person offset (behind player)
        thirdPerson(true) // Start in first-person mode
    {
    }

    void addModel(Model* entityModel = nullptr) {
        model = entityModel;
    }

    void processKeyboard(std::unordered_set<int> pressedKeys, float deltaTime) {
        glm::vec3 force = glm::vec3(0.0f);
        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        // Process continuous movement
        if (pressedKeys.count(GLFW_KEY_W)) force += flatFront * movementSpeed;
        if (pressedKeys.count(GLFW_KEY_S)) force -= flatFront * movementSpeed;
        if (pressedKeys.count(GLFW_KEY_A)) force -= right * movementSpeed;
        if (pressedKeys.count(GLFW_KEY_D)) force += right * movementSpeed;
        if (pressedKeys.count(GLFW_KEY_SPACE)) jump(10.0f);

        applyForce(force);
        if (glm::length(force) > 0.001f) {
            float yawOffset = camYaw - yaw;

            // Normalize to [-180, 180] range for smooth rotation
            if (yawOffset > 180.0f) yawOffset -= 360.0f;
            if (yawOffset < -180.0f) yawOffset += 360.0f;

            rotate(yawOffset, 0.0f);
        }

    }

    void processMouseMovement(float xpos, float ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camYaw += xoffset;
        camPitch += yoffset;

        camPitch = std::clamp(camPitch, -89.0f, 89.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        direction.y = sin(glm::radians(camPitch));
        direction.z = sin(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        cameraFront = glm::normalize(direction);
    }

    void swapViewMode() {
        thirdPerson = !thirdPerson;
        if (model) {
            if (thirdPerson) { model->alpha = 1; }
            else { model->alpha = 0; }
        }
    }

    // Compute the View Matrix for rendering
    glm::mat4 getViewMatrix() {
        if (thirdPerson) {
            glm::vec3 camPos = position - cameraFront * thirdPersonOffset.z + glm::vec3(0.0f, thirdPersonOffset.y, 0.0f);
            return glm::lookAt(camPos, position + cameraFront, up);
        }
        else {
            return glm::lookAt(position + viewpointOffset, position + viewpointOffset + cameraFront, up);
        }
    }


    glm::vec3 getEfPos() {
        if (thirdPerson) {
            return position - front * thirdPersonOffset.z + glm::vec3(0.0f, thirdPersonOffset.y, 0.0f);
        }
        else {
            return position + viewpointOffset;
        }
    }
};
