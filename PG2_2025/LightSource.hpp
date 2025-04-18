#pragma once
#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class LightSource {
public:
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 ambient = glm::vec3(0.1f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(1.0f);

    virtual void apply(GLuint shaderID, int index) const = 0; // each light applies itself
    virtual std::string getType() const = 0; // "directional", "point", etc.
    virtual ~LightSource() = default;
};

class SpotLight : public LightSource {
public:
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    void apply(GLuint shaderID, int index) const override {
        std::string prefix = "spotLights[" + std::to_string(index) + "]";
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".position").c_str()), 1, glm::value_ptr(position));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".direction").c_str()), 1, glm::value_ptr(direction));
        glUniform1f(glGetUniformLocation(shaderID, (prefix + ".cutOff").c_str()), cutOff);
        glUniform1f(glGetUniformLocation(shaderID, (prefix + ".outerCutOff").c_str()), outerCutOff);
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".color").c_str()), 1, glm::value_ptr(color));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".specular").c_str()), 1, glm::value_ptr(specular));
    }

    std::string getType() const override { return "spot"; }
};

class DirectionalLight : public LightSource {
public:
    glm::vec3 direction;

    void apply(GLuint shaderID, int index) const override {
        std::string prefix = "dirLights[" + std::to_string(index) + "]";
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".direction").c_str()), 1, glm::value_ptr(direction));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".color").c_str()), 1, glm::value_ptr(color));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".specular").c_str()), 1, glm::value_ptr(specular));
    }

    std::string getType() const override { return "directional"; }
};

class PointLight : public LightSource {
public:
    glm::vec3 position;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    void apply(GLuint shaderID, int index) const override {
        std::string prefix = "pointLights[" + std::to_string(index) + "]";
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".position").c_str()), 1, glm::value_ptr(position));
        glUniform1f(glGetUniformLocation(shaderID, (prefix + ".constant").c_str()), constant);
        glUniform1f(glGetUniformLocation(shaderID, (prefix + ".linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(shaderID, (prefix + ".quadratic").c_str()), quadratic);
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".color").c_str()), 1, glm::value_ptr(color));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderID, (prefix + ".specular").c_str()), 1, glm::value_ptr(specular));
    }

    std::string getType() const override { return "point"; }
};

class AmbientLight : public LightSource {
public:
    void apply(GLuint shaderID, int /*index*/) const override {
        glUniform3fv(glGetUniformLocation(shaderID, "ambientLightColor"), 1, glm::value_ptr(color));
    }

    std::string getType() const override { return "ambient"; }
};
