// Author: JJ

//
// WARNING:
// In general, you can NOT freely reorder includes!
//

// C++
// include anywhere, in any order
#include <iostream>
#include <chrono>
#include <stack>
#include <random>

// OpenCV (does not depend on GL)
#include <opencv2\opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h> 
// WGLEW = Windows GL Extension Wrangler (change for different platform) 
// platform specific functions (in this case Windows)
#include <GL/wglew.h> 

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// icp.cpp 
// author: JJ
#include "gl_err_callback.h"
#include "app.hpp"
#include "ShaderProgram.hpp"
#include "Model.hpp"

GLFWwindow* window = nullptr;
App::App()
{
    // default constructor
    // nothing to do here (so far...)
    std::cout << "Constructed...\n";
    
}
App::~App()
{
    //cleanup GL data
    glDeleteProgram(shader_prog_ID);
    glDeleteBuffers(1, &VBO_ID);
    glDeleteVertexArrays(1, &VAO_ID);
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
    // move to App destructor
    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();
    

    exit(EXIT_SUCCESS);
}

bool App::init(int aa = 1)
{
    try {
        // Step 1: Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Error: Failed to initialize GLFW\n";
            return false;
        }

        // Step 2: Create a temporary OpenGL context (for extensions)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        GLFWwindow* tempWindow = glfwCreateWindow(100, 100, "Temp Context", NULL, NULL);
        if (!tempWindow) {
            std::cerr << "Error: Failed to create temporary GLFW window\n";
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(tempWindow);

        // Step 3: Initialize GLEW for temporary context
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Error: GLEW init failed!\n";
            glfwDestroyWindow(tempWindow);
            glfwTerminate();
            return false;
        }
        glfwWindowHint(GLFW_SAMPLES, aa);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL);

        // Step 4: Destroy temporary window & request final OpenGL context
        glfwDestroyWindow(tempWindow);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Ensure compatibility

        glfwWindowHint(GLFW_DEPTH_BITS, 24);

        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        window = glfwCreateWindow(mode->width, mode->height, "OpenGL Context", primaryMonitor, NULL);

        if (!window) {
            std::cerr << "Error: Failed to create main GLFW window\n";
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);
        if (!glfwGetCurrentContext()) {
            std::cerr << "Error: OpenGL context is NULL!\n";
            return false;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Step 5: Initialize GLEW (again, for the real context)
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Error: GLEW re-init failed!\n";
            glfwTerminate();
            return false;
        }

        wglewInit(); // Initialize WGLEW after GLEW

        // Step 6: Setup OpenGL Debug Output
        if (GLEW_ARB_debug_output) {
            glDebugMessageCallback(MessageCallback, 0);
            glEnable(GL_DEBUG_OUTPUT);
            std::cout << "GL_DEBUG enabled." << std::endl;
        }
        else {
            std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
        }

        // Step 7: Ensure OpenGL Core Profile supports DSA (Direct State Access)
        if (!GLEW_ARB_direct_state_access) {
            std::cerr << "Error: No Direct State Access (DSA) support\n";
            return false;
        }

        // Step 8: Configure V-Sync
        App::vsync_on = true;
        glfwSwapInterval((vsync_on ? 1 : 0));

        // Add mouse callback to control camera
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and lock cursor
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetWindowUserPointer(window, this);

        // Add camera callback
        glfwSetKeyCallback(window, key_callback);
        glfwSetWindowUserPointer(window, this);


        // Step 9: Initialize OpenGL assets



        std::cout << "Initialized...\n";
        return true;
    }
    catch (std::exception const& e) {
        std::cerr << "Init failed: " << e.what() << std::endl;
        glfwTerminate();
        throw;
    }
}

void App::init_hm()
{
    std::filesystem::path hm_file("assets/heights.png");
    cv::Mat hmap = cv::imread(hm_file.string(), cv::IMREAD_GRAYSCALE);

    if (hmap.empty()) {
        throw std::runtime_error("ERR: Height map empty? File: " + hm_file.string());
    }

    height_map = MapGen::GenHeightMap(hmap, 10, heightScale);
    std::cout << "Note: Heightmap vertices: " << height_map.vertices.size() << std::endl;
}

GLuint App::textureInit(const std::filesystem::path& file_name)
{
    cv::Mat image = cv::imread(file_name.string(), cv::IMREAD_UNCHANGED);  // Load with alpha if present
    if (image.empty()) {
        throw std::runtime_error("No texture found in file: " + file_name.string());
    }

    return gen_tex(image);
}

GLuint App::gen_tex(cv::Mat& image)
{
    GLuint ID = 0;

    if (image.empty()) {
        throw std::runtime_error("Image is empty?");
    }

    // Convert grayscale to RGB if needed
    if (image.channels() == 1) {
        cv::Mat converted;
        cv::cvtColor(image, converted, cv::COLOR_GRAY2RGB);
        image = converted;  // Replace with the new 3-channel image
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &ID);

    switch (image.channels()) {
    case 3:
        glTextureStorage2D(ID, 1, GL_RGB8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_RGB, GL_UNSIGNED_BYTE, image.data);
        break;
    case 4:
        glTextureStorage2D(ID, 1, GL_RGBA8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
        break;
    default:
        throw std::runtime_error("Unsupported number of channels in texture: " + std::to_string(image.channels()));
    }

    // Mipmap generation and filtering
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateTextureMipmap(ID);

    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return ID;
}

void App::init_assets(void) {
    //
    // Initialize pipeline: compile, link and use shaders
    //

    // SHADERS - define & compile & link
    ShaderProgram my_shader("assets/shaders/01_shaded_sample/basic.vert",
        "assets/shaders/01_shaded_sample/basic.frag");
    shader_prog_ID = my_shader.getID();
    if (!glIsProgram(shader_prog_ID)) {
        std::cerr << "Error: Shader program is invalid!" << std::endl;
        return;
    }
    glUseProgram(shader_prog_ID);

    // Load player model
    Model* playerModel = new Model("assets/obj/teapot_tri_vnt.obj", my_shader);
    //camera.addModel(playerModel);

    scene.emplace("plane", Model("assets/obj/plane_tri_vnt.obj", my_shader));

    Model* teapotModel = new Model("assets/obj/teapot_tri_vnt.obj", my_shader);
    GLuint tex = textureInit("assets/box.png");
    teapotModel->setTexture(tex);
    entities.emplace("teapot", Entity(glm::vec3(20, 5, 5), teapotModel));

    Model* FishModel = new Model("assets/obj/fish.obj", my_shader);
    entities.emplace("fish", Entity(glm::vec3(5, 5, 5), FishModel));

    // Create and load data into GPU using OpenGL DSA (Direct State Access)
    glCreateVertexArrays(1, &VAO_ID);
    glBindVertexArray(VAO_ID);

    // Retrieve attribute locations
    GLint position_attrib_location = glGetAttribLocation(my_shader.getID(), "aPos");

    if (position_attrib_location == -1) {
        std::cerr << "Error: Could not find 'aPos' in shader program." << std::endl;
    }

    // Enable position attribute
    glEnableVertexArrayAttrib(VAO_ID, position_attrib_location);
    glVertexArrayAttribFormat(VAO_ID, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
    glVertexArrayAttribBinding(VAO_ID, position_attrib_location, 0);

    // Create and fill data
    glCreateBuffers(1, &VBO_ID);
    glNamedBufferData(VBO_ID, triangle_vertices.size() * sizeof(Vertex), triangle_vertices.data(), GL_STATIC_DRAW);

    // Connect together
    glVertexArrayVertexBuffer(VAO_ID, 0, VBO_ID, 0, sizeof(Vertex));

    std::cout << "Triangle Vertices Count: " << triangle_vertices.size() << std::endl;
    if (triangle_vertices.empty()) {
        std::cerr << "Error: No vertex data loaded!\n";
    }
    for (size_t i = 0; i < std::min(triangle_vertices.size(), size_t(3)); i++) {
        std::cout << "Vertex " << i << ": ("
            << triangle_vertices[i].Position.x << ", "
            << triangle_vertices[i].Position.y << ", "
            << triangle_vertices[i].Position.z << ")\n";
    }

}

int App::run(void)
{
    try {
        r = 0.0f;
        g = 0.0f;
        b = 1.0f;
        a = 1.0f;

        // Activate shader program
        glUseProgram(shader_prog_ID);

        // Get uniform location in GPU program
        GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
        if (uniform_color_location == -1) {
            std::cerr << "Uniform location is not found in active shader program. Did you forget to activate it?\n";
        }

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // RED background
        glEnable(GL_DEPTH_TEST); // Enable depth testing


        float lastFrame = glfwGetTime();

        while (!glfwWindowShouldClose(window))
        {
            float currentFrame = glfwGetTime();
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            camera.update(deltaTime, 1);
            camera.processKeyboard(pressedKeys, deltaTime);
            for (auto& [name, entity] : entities) {
                entity.update(deltaTime, 1);
            }

            // FPS calculations
            crntTime = glfwGetTime();
            timeDiff = crntTime - prevTime;
            counter++;
            if (timeDiff >= 1.0) {
                std::string FPS = std::to_string((1.0 / timeDiff) * counter) + " FPS";
                std::string vsync_status = "Vsync: " + std::string((vsync_on) ? "On" : "Off");
                glfwSetWindowTitle(window, (FPS + " " + vsync_status).c_str());
                prevTime = crntTime;
                counter = 0;
            }

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set uniform color
            glUniform4f(uniform_color_location, r, g, b, a);

            // Camera transformation: Update `view` based on movement
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            glm::mat4 view = camera.getViewMatrix(); // Get updated camera view

            // Set transformation matrix (uMVP) for the model
            //glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, -5.0f, -10.0f)); // Move it down and back
            glm::mat4 uMVP = projection * view;

            GLint mvpLoc = glGetUniformLocation(shader_prog_ID, "uMVP");
            if (mvpLoc != -1) {
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(uMVP));
            }
            else {
                std::cerr << "ERROR: uMVP uniform not found in shader!" << std::endl;
            }

            height_map.draw(projection, view);


            // Iterate through the scene and render each model using `model.draw()`
            for (auto& [name, model] : scene) {
                model.draw(projection, view);
            }

            // Render Dynamic Entities (Entities)
            for (auto& [name, entity] : entities) {
                //entity.jump(10);
                //entity.rotate(5, 0);
                if (!entity.model->transparent) {
                    entity.render(shader_prog_ID, projection, view);
                }
                else {
                    transparent.push_back(&entity);
                }
            }

            std::sort(transparent.begin(), transparent.end(), [&](Entity const* a, Entity const* b) {
                return glm::distance(camera.position, a->position) > glm::distance(camera.position, b->position);
                });



            for (auto& [name, entity] : entities) {
                entity.model->transparent = true;
                entity.render(shader_prog_ID, projection, view);
            }

            camera.render(shader_prog_ID, projection, view);

            // Poll events and swap buffers
            glfwPollEvents();
            glfwSwapBuffers(window);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Finished OK...\n";
    return EXIT_SUCCESS;
}

void App::error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (this_inst->r >= 1.0) {
            this_inst->r = 0.0;
            this_inst->g = 1.0;
            this_inst->b = 0.0;
        }
        else if (this_inst->g >= 1.0) {
            this_inst->r = 0.0;
            this_inst->g = 0.0;
            this_inst->b = 1.0;
        }
        else if (this_inst->b >= 1.0) {
            this_inst->r = 1.0;
            this_inst->g = 0.0;
            this_inst->b = 0.0;
        }
    }
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->camera.processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
    }
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (yoffset > 0.0) {
        std::cout << "wheel up...\n";
    }
    this_inst->g += 0.05;
    if (this_inst->g >= 1.0) {
        this_inst->g = 0;
    }
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!this_inst) return;
    int samples = 0;

    if (action == GLFW_PRESS) {
        this_inst->pressedKeys.insert(key);
    }
    else if (action == GLFW_RELEASE) {
        this_inst->pressedKeys.erase(key);
    }

    // Handle one-time key actions
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            this_inst->vsync_on = !this_inst->vsync_on;
            glfwSwapInterval(this_inst->vsync_on);
            break;
        case GLFW_KEY_B:
            this_inst->camera.swapViewMode();
            break;
        case GLFW_KEY_N:
            glGetIntegerv(GL_SAMPLES, &samples);
            switch (samples) {
            case 1: samples = 2; break;
            case 2: samples = 4; break;
            case 4: samples = 8; break;
            case 8: samples = 1; break;
            }
            glfwWindowHint(GLFW_SAMPLES, samples);
            //this_inst->init(samples);
            std::cout << "Switching AA to: " << samples << "x\n";
        default:
            break;
        }
    }
}


