#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <future>
#include <iostream>
#include <mutex>

#include "Vertex.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    ShaderProgram shader{};
    std::mutex load_mutex;
    GLuint texture_id = 0;
    bool transparent{ false };

    // Constructor
    Model(const std::filesystem::path& filename, ShaderProgram& shader)
        : shader(shader) {
        loadModelAsync(filename);
    }

    // Delete Copy Constructor & Assignment
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    // Move Constructor
    Model(Model&& other) noexcept
        : meshes(std::move(other.meshes)),
        name(std::move(other.name)),
        origin(other.origin),
        orientation(other.orientation),
        shader(other.shader) { // No std::move() for references!
    }

    // chybìl inicializátor
    // Move Assignment Operator
    Model& operator=(Model&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock(other.load_mutex); // Thread safety
            meshes = std::move(other.meshes);
            name = std::move(other.name);
            origin = other.origin;
            orientation = other.orientation;
            shader = std::move(other.shader);
        }
        return *this;
    }

    void setTexture(GLuint tex) {
        texture_id = tex;
    }

    void draw(const glm::mat4& projection, const glm::mat4& view,
        const glm::vec3& offset = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f)) {
        std::lock_guard<std::mutex> lock(load_mutex);

        glUseProgram(shader.getID());

        if (texture_id != 0) {
            glBindTextureUnit(0, texture_id);
            glUniform1i(glGetUniformLocation(shader.getID(), "tex0"), 0);
        }

        for (auto& mesh : meshes) {
            mesh.draw(projection, view, origin + offset, orientation + rotation);
        }
    }



private:
    void loadModelAsync(const std::filesystem::path& path) {
        std::cout << "Debug: Starting async model load for " << path << std::endl;
        loadModel(path); // Directly call loadModel instead of deferring
    }


    void loadModel(const std::filesystem::path& path) {
        // Check if the shader is valid before using it
        if (!glIsProgram(shader.getID())) {
            std::cerr << "Error: Shader program is invalid in loadModel!\n";
            return;
        }
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        if (!loadOBJ(path.string().c_str(), vertices, uvs, normals)) {
            std::cerr << "Error loading OBJ file: " << path << std::endl;
            return; 
        }

        std::cout << "Loaded OBJ: " << path << std::endl;
        std::cout << "   Vertices from file: " << vertices.size() << std::endl;
        std::cout << "   UVs: " << uvs.size() << std::endl;
        std::cout << "   Normals: " << normals.size() << std::endl;

        std::lock_guard<std::mutex> lock(load_mutex);
        std::vector<Vertex> vertexData;
        std::vector<unsigned int> indices;

        for (size_t i = 0; i < vertices.size(); ++i) {
            Vertex vertex;
            vertex.Position = vertices[i];
            if (!normals.empty()) vertex.Normal = normals[i];
            if (!uvs.empty()) vertex.TexCoords = uvs[i];
            vertexData.push_back(vertex);
            indices.push_back(static_cast<unsigned int>(i));
        }

        std::cout << "   Final Vertex Count: " << vertexData.size() << std::endl;
        std::cout << "   Index Count: " << indices.size() << std::endl;

        if (vertexData.empty()) {
            std::cerr << "Error: No vertex data loaded!" << std::endl;
            return;
        }

        meshes.emplace_back(GL_TRIANGLES, shader, vertexData, indices, origin, orientation);
    }
};
