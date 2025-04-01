#include "mapgen.hpp"
#include "app.hpp"
#include <iostream>
#include <algorithm>
#include <initializer_list>

Mesh MapGen::GenHeightMap(const cv::Mat& hmap, unsigned int mesh_step_size, float heightScale)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    std::cout << "Note: Heightmap size: " << hmap.cols << "x" << hmap.rows << ", channels: " << hmap.channels() << std::endl;

    for (unsigned int x = 0; x < (hmap.cols - mesh_step_size); x += mesh_step_size)
    {
        for (unsigned int z = 0; z < (hmap.rows - mesh_step_size); z += mesh_step_size)
        {
            float h0 = hmap.at<uchar>(cv::Point(x, z)) / 255.0f;
            float h1 = hmap.at<uchar>(cv::Point(x + mesh_step_size, z)) / 255.0f;
            float h2 = hmap.at<uchar>(cv::Point(x + mesh_step_size, z + mesh_step_size)) / 255.0f;
            float h3 = hmap.at<uchar>(cv::Point(x, z + mesh_step_size)) / 255.0f;

            float x_offset = (hmap.cols - mesh_step_size) / 2.0f;
            float z_offset = (hmap.rows - mesh_step_size) / 2.0f;

            glm::vec3 p0(x - x_offset, h0 * heightScale, z - z_offset);
            glm::vec3 p1(x + mesh_step_size - x_offset, h1 * heightScale, z - z_offset);
            glm::vec3 p2(x + mesh_step_size - x_offset, h2 * heightScale, z + mesh_step_size - z_offset);
            glm::vec3 p3(x - x_offset, h3 * heightScale, z + mesh_step_size - z_offset);


            float max_h = *std::max_element(std::initializer_list<float>{h0, h1, h2, h3}.begin(),
                std::initializer_list<float>{h0, h1, h2, h3}.end());


            glm::vec2 tc0 = MapGen::get_subtex_by_height(max_h);
            glm::vec2 tc1 = tc0 + glm::vec2(1.0f / 16, 0.0f);
            glm::vec2 tc2 = tc0 + glm::vec2(1.0f / 16, 1.0f / 16);
            glm::vec2 tc3 = tc0 + glm::vec2(0.0f, 1.0f / 16);

            glm::vec3 n1 = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            glm::vec3 n2 = glm::normalize(glm::cross(p2 - p0, p3 - p0));
            glm::vec3 navg = glm::normalize(n1 + n2);

            vertices.emplace_back(Vertex{ p0, navg, tc0 });
            vertices.emplace_back(Vertex{ p1, n1, tc1 });
            vertices.emplace_back(Vertex{ p2, navg, tc2 });
            vertices.emplace_back(Vertex{ p3, n2, tc3 });

            indices.push_back(vertices.size() - 4);
            indices.push_back(vertices.size() - 3);
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 4);
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 1);
        }
    }

    GLuint texID = App::textureInit("assets/textures/heights.png");

    ShaderProgram shader("assets/shaders/terrain.vert", "assets/shaders/terrain.frag");

    return Mesh(GL_TRIANGLES, shader, vertices, indices, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), texID);
}

glm::vec2 MapGen::get_subtex_st(int x, int y) {
    return glm::vec2(x * 1.0f / 16, y * 1.0f / 16);
}

glm::vec2 MapGen::get_subtex_by_height(float height) {
    if (height > 0.9)
        return get_subtex_st(2, 11);
    else if (height > 0.8)
        return get_subtex_st(3, 11);
    else if (height > 0.5)
        return get_subtex_st(0, 14);
    else if (height > 0.3)
        return get_subtex_st(2, 15);
    else
        return get_subtex_st(0, 11);
}