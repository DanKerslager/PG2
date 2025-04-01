#pragma once
#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "ShaderProgram.hpp"
#include "Vertex.hpp"

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

        glBindVertexArray(0);
    }

    //todo
    void draw(const glm::mat4& projection, const glm::mat4& view,
        const glm::vec3& offset = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f)) {
        if (VAO == 0) {
            std::cerr << "VAO not initialized!\n";
            return;
        }

        shader.activate();

        if (!glIsProgram(shader.getID())) {
            std::cerr << "Error: Shader program is invalid!\n";
            return;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), origin + offset) *
            glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));

        glm::mat4 mvp = projection * view * model;
        GLint modelLoc = glGetUniformLocation(shader.getID(), "uMVP");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        if (texture_id > 0) {
            glBindTextureUnit(0, texture_id);
            glUniform1i(glGetUniformLocation(shader.getID(), "tex0"), 0);
        }

        glBindVertexArray(VAO);
        glDrawElements(primitive_type, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
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
};
