// icp.cpp 
// author: JJ

#pragma once
#include "Vertex.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <unordered_set>
#include "Model.hpp"
#include "Camera.hpp"
#include "entity.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>  // Ensure core OpenCV components are included
#include "mapgen.hpp"
#include "LightSource.hpp"


class App {
private:
    //new GL stuff
    GLuint shader_prog_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint VAO_ID{ 0 };

    GLfloat r{ 1.0f }, g{ 0.0f }, b{ 0.0f }, a{ 1.0f };

    std::vector<Vertex> triangle_vertices;
    bool vsync_on;
    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;    
    unsigned int counter = 0;
    std::unordered_map<std::string, Model> scene;
    std::unordered_map<std::string, Entity*> entities;
    std::vector<Entity*> transparent;


    float heightScale = 50.0f;
    Mesh height_map;
public:
    App();
    static GLuint textureInit(const std::filesystem::path& file_name);
    static GLuint gen_tex(cv::Mat& image);
    void init_hm();
    std::vector<LightSource*> lights;

    static int aa;
    bool debug;
    bool fullscreen;
    int windowPosX, windowPosY, windowWidth, windowHeight;

    bool init(int aa);
    void init_assets(void);
    int run(void);
    void error_callback(int error, const char* description);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    int loadAASamplesFromConfig(const std::string& path);

    ~App();
private:
    Camera camera{ glm::vec3(0.0f, 0.0f, 5.0f) }; // Start camera at (0,0,5)

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    std::unordered_set<int> pressedKeys; // Tracks active keys
};

