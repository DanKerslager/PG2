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

    Camera(glm::vec3 startPosition, Model* entityModel = nullptr)
        : Entity(startPosition, entityModel), sensitivity(0.1f), firstMouse(true),
        lastX(400), lastY(300),
        viewpointOffset(glm::vec3(0.0f, 3.0f, 0.0f)), // First-person offset (head position)
        thirdPersonOffset(glm::vec3(0.0f, 10.0f, 25.0f)), // Third-person offset (behind player)
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
    }

    void processMouseMovement(float xpos, float ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Reversed since Y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        rotate(xoffset, yoffset);
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
            return glm::lookAt(position - front * thirdPersonOffset.z + glm::vec3(0.0f, thirdPersonOffset.y, 0.0f),
                position + front, up);
        }
        else {
            return glm::lookAt(position + viewpointOffset, position + front + viewpointOffset, up);
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
