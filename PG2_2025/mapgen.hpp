#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include "Mesh.hpp"

class MapGen {
public:
    static Mesh GenHeightMap(const cv::Mat& hmap, unsigned int mesh_step_size, float heightScale);
    static glm::vec2 get_subtex_by_height(float height);
    static glm::vec2 get_subtex_st(int x, int y);
};
