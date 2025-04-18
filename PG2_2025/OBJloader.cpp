#include <string>
#include <GL/glew.h> 
#include <glm/glm.hpp>

#include "OBJloader.hpp"

#define MAX_LINE_SIZE 255

bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals)
{
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    struct VertexIndex {
        unsigned int v, vt, vn;
    };

    std::vector<VertexIndex> final_indices;

    out_vertices.clear();
    out_uvs.clear();
    out_normals.clear();

    FILE* file;
    fopen_s(&file, path, "r");
    if (!file) {
        printf("Cannot open file: %s\n", path);
        return false;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            glm::vec3 vertex;
            sscanf_s(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strncmp(line, "vt ", 3) == 0) {
            glm::vec2 uv;
            sscanf_s(line, "vt %f %f", &uv.y, &uv.x);  // flipped Y
            temp_uvs.push_back(uv);
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            glm::vec3 normal;
            sscanf_s(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strncmp(line, "f ", 2) == 0) {
            std::vector<VertexIndex> faceIndices;
            char* context = nullptr;
            char* tok = strtok_s(line + 2, " \n", &context);
            while (tok) {
                VertexIndex idx = { 0, 0, 0 };
                sscanf_s(tok, "%u/%u/%u", &idx.v, &idx.vt, &idx.vn);
                faceIndices.push_back(idx);
                tok = strtok_s(NULL, " \n", &context);
            }

            // Fan triangulation: [0, i, i+1]
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                final_indices.push_back(faceIndices[0]);
                final_indices.push_back(faceIndices[i]);
                final_indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    for (auto& idx : final_indices) {
        out_vertices.push_back(temp_vertices[idx.v - 1]);
        out_uvs.push_back(idx.vt ? temp_uvs[idx.vt - 1] : glm::vec2(0.0f));
        out_normals.push_back(idx.vn ? temp_normals[idx.vn - 1] : glm::vec3(0.0f, 1.0f, 0.0f));
    }

    fclose(file);
    return true;
}
