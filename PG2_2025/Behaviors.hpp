#pragma once

#include "Entity.hpp"
#include <glm/glm.hpp>
#include <functional>
#include <cmath>

namespace Behaviors {

    using Behavior = Entity::Behavior;

    // Walk in a circle using applyForce()
    inline Behavior WalkInCircle(glm::vec3 center, float radius, float speed = 1.0f) {
        float angle = 0.0f;

        return [=](Entity& self, float dt) mutable {
            angle += speed * dt;
            glm::vec3 target;
            target.x = center.x + cos(angle) * radius;
            target.z = center.z + sin(angle) * radius;
            target.y = self.position.y; // keep y unchanged

            glm::vec3 dir = glm::normalize(target - self.position);
            self.applyForce(dir * self.movementSpeed);
            };
    }

    // Bob up and down (could be decorative, not physics)
    inline Behavior Bob(float amplitude = 0.5f, float speed = 1.0f) {
        float baseY = 0.0f;
        bool firstRun = true;

        return [=](Entity& self, float dt) mutable {
            if (firstRun) {
                baseY = self.position.y;
                firstRun = false;
            }
            self.position.y = baseY + sin(glfwGetTime() * speed) * amplitude;
            };
    }

    // Move forward based on current orientation
    inline Behavior MoveForward(float speed = 1.0f) {
        return [=](Entity& self, float dt) {
            glm::vec3 horizontalFront = glm::normalize(glm::vec3(self.front.x, 0.0f, self.front.z));
            self.applyForce(horizontalFront * speed);
            };
    }

    // Jump on a timer (e.g., every 2 seconds)
    inline Behavior PeriodicJump(float strength = 5.0f, float interval = 2.0f) {
        float timer = 0.0f;
        return [=](Entity& self, float dt) mutable {
            timer += dt;
            if (timer >= interval) {
                self.jump(strength);
                timer = 0.0f;
            }
            };
    }

    // Idle spin (rotate yaw)
    inline Behavior Spin(float degreesPerSecond = 90.0f) {
        return [=](Entity& self, float dt) {
            self.rotate(degreesPerSecond * dt, 0.0f);
            };
    }

}
