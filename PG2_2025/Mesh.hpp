#pragma once
#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp> 
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderProgram.hpp"
#include "Vertex.hpp"
#include "LightSource.hpp"


class Mesh {
public:
    // Mesh data
    glm::vec3 origin{};
    glm::vec3 orientation{};

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    GLuint texture_id{ 0 }; // Texture ID = 0 means no texture
    GLenum primitive_type = GL_POINT;
    ShaderProgram shader;

    struct UniformLocations {
        GLint uMVP = -1;
        GLint uModel = -1;
        GLint alpha = -1;
        GLint ambientColor = -1;
        GLint diffuseColor = -1;
        GLint specularColor = -1;
        GLint shininess = -1;
        GLint viewPos = -1;
        GLint tex0 = -1;

        GLint numDirLights = -1;
        GLint numSpotLights = -1;
        GLint numPointLights = -1;
    };

    UniformLocations uniforms;
    GLuint id;

    // Mesh material
    glm::vec4 ambient_material{ 1.0f }; // White, non-transparent 
    glm::vec4 diffuse_material{ 1.0f }; // White, non-transparent 
    glm::vec4 specular_material{ 1.0f }; // White, non-transparent
    float reflectivity{ 1.0f };

    Mesh() = default;

    // Indirect (indexed) draw 
    Mesh(GLenum primitive_type, ShaderProgram& shader,
        std::vector<Vertex> const& vertices,
        std::vector<GLuint> const& indices,
        glm::vec3 const& origin, glm::vec3 const& orientation,
        GLuint const texture_id = 0)
        : primitive_type(primitive_type),
        shader(shader),
        vertices(vertices),
        indices(indices),
        origin(origin),
        orientation(orientation),
        texture_id(texture_id) {

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));


        glBindVertexArray(0);

        cacheUniformLocations();
    }


    void draw(const glm::mat4& projection, const glm::mat4& view,
        const std::vector<LightSource*> lights,
        const glm::vec3& offset = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const float alpha = 1.0f) {
        if (VAO == 0) {
            std::cerr << "VAO not initialized!\n";
            return;
        }

        shader.activate();

        if (!glIsProgram(shader.getID())) {
            std::cerr << "Error: Shader program is invalid!\n";
            return;
        }

        // === TRANSFORMS ===
        glm::quat q = glm::quat(glm::radians(rotation));
        glm::mat4 rotationMatrix = glm::mat4_cast(q);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), origin + offset) * rotationMatrix;

        glm::mat4 mvp = projection * view * model;

        // === UNIFORMS ===
        glUniformMatrix4fv(uniforms.uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(uniforms.uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(uniforms.alpha, alpha);

		applyLights(lights);

        // Material properties
        glUniform3f(uniforms.ambientColor, 0.2f, 0.2f, 0.2f);
        glUniform3f(uniforms.diffuseColor, 0.8f, 0.8f, 0.8f);
        glUniform3f(uniforms.specularColor, 1.0f, 1.0f, 1.0f);
        glUniform1f(uniforms.shininess, 32.0f);

        // Pass camera position (reverse-transform from view matrix)
        glm::vec3 cameraPosition = glm::vec3(glm::inverse(view)[3]);
        glUniform3fv(uniforms.viewPos, 1, glm::value_ptr(cameraPosition));

        // Texture binding
        if (texture_id > 0) {
            glBindTextureUnit(0, texture_id);
            glUniform1i(uniforms.tex0, 0);
        }

        // === DRAW ===
        glBindVertexArray(VAO);
        glDrawElements(primitive_type, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

	void applyLights(const std::vector<LightSource*>& lights) {
        // Lights
        int dirIndex = 0, spotIndex = 0, pointIndex = 0;
        bool ambientSet = false;

        for (LightSource* light : lights) {
            std::string type = light->getType();

            if (type == "directional") {
                light->apply(shader.getID(), dirIndex++);
            }
            else if (type == "spot") {
                light->apply(shader.getID(), spotIndex++);
            }
            else if (type == "point") {
                light->apply(shader.getID(), pointIndex++);
            }
            else if (type == "ambient" && !ambientSet) {
                light->apply(shader.getID(), 0); // only once
                ambientSet = true;
            }
        }

        // Light counts (optional, but recommended for the shader)
        glUniform1i(uniforms.numDirLights, dirIndex);
        glUniform1i(uniforms.numSpotLights, spotIndex);
        glUniform1i(uniforms.numPointLights, pointIndex);
	}

    float getHeightAt(const glm::vec3& position) const {
        float closestDist = 1e30f;
        float closestHeight = 0.0f;

        for (const auto& v : vertices) {
            float dx = v.Position.x - position.x;
            float dz = v.Position.z - position.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < closestDist) {
                closestDist = distSq;
                closestHeight = v.Position.y;
            }
        }

        return closestHeight;
    }

    void clear(void) {
        texture_id = 0;
        primitive_type = GL_POINT;
        ambient_material = glm::vec4(1.0f);
        diffuse_material = glm::vec4(1.0f);
        specular_material = glm::vec4(1.0f);
        reflectivity = 1.0f;

        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
        if (EBO != 0) {
            glDeleteBuffers(1, &EBO);
            EBO = 0;
        }
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
    }

private:
    // OpenGL buffer IDs
    unsigned int VAO{ 0 }, VBO{ 0 }, EBO{ 0 };

    void cacheUniformLocations() {
        id = shader.getID();

        uniforms.uMVP = glGetUniformLocation(id, "uMVP");
        uniforms.uModel = glGetUniformLocation(id, "uModel");
        uniforms.alpha = glGetUniformLocation(id, "alpha");
        uniforms.ambientColor = glGetUniformLocation(id, "ambientColor");
        uniforms.diffuseColor = glGetUniformLocation(id, "diffuseColor");
        uniforms.specularColor = glGetUniformLocation(id, "specularColor");
        uniforms.shininess = glGetUniformLocation(id, "shininess");
        uniforms.viewPos = glGetUniformLocation(id, "viewPos");
        uniforms.tex0 = glGetUniformLocation(id, "tex0");

        uniforms.numDirLights = glGetUniformLocation(id, "numDirLights");
        uniforms.numSpotLights = glGetUniformLocation(id, "numSpotLights");
        uniforms.numPointLights = glGetUniformLocation(id, "numPointLights");
    }

};
