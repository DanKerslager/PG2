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
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

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
#include "Behaviors.hpp"
#include "Particles.hpp"

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

bool App::init(int aa)
{
    try {
        // Step 0: Load settings
        if (aa <= 0) {
            aa = loadAASamplesFromConfig("settings.json");
        }

        // Step 1: Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Error: Failed to initialize GLFW\n";
            return false;
        }

        // Step 2: Request core profile OpenGL context directly
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For macOS compatibility
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_SAMPLES, aa);

        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        // === Windowed mode defaults ===
        windowWidth = 1280;
        windowHeight = 720;
        windowPosX = 100;
        windowPosY = 100;
        fullscreen = false;

        // Create window in windowed mode
        window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Context", nullptr, nullptr);
        if (!window) {
            std::cerr << "Error: Failed to create GLFW window\n";
            glfwTerminate();
            return false;
        }
        glfwSetWindowPos(window, windowPosX, windowPosY); // Optional: place on screen
        glfwMakeContextCurrent(window);

        glfwMakeContextCurrent(window);
        if (!glfwGetCurrentContext()) {
            std::cerr << "Error: OpenGL context is NULL!\n";
            return false;
        }

        // Step 3: Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Error: GLEW init failed!\n";
            glfwTerminate();
            return false;
        }

        wglewInit(); // Optional, only needed for some Windows-specific extensions

        // Step 4: Setup OpenGL Debug Output
        if (GLEW_ARB_debug_output) {
            glDebugMessageCallback(MessageCallback, 0);
            glEnable(GL_DEBUG_OUTPUT);
            std::cout << "GL_DEBUG enabled." << std::endl;
        }
        else {
            std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
        }

        // Step 5: Ensure DSA support
        if (!GLEW_ARB_direct_state_access) {
            std::cerr << "Error: No Direct State Access (DSA) support\n";
            return false;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL);

        // Step 6: Configure V-Sync
        App::vsync_on = true;
        glfwSwapInterval((vsync_on ? 1 : 0));

        // Step 7: Input callbacks
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetWindowUserPointer(window, this);

        // Step 8: Texture setup defaults
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Step 9: Get framebuffer size and set viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
            glViewport(0, 0, width, height);
            });
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

    height_map = MapGen::GenHeightMap(hmap, 2, heightScale);
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

    scene.emplace("plane", Model("assets/obj/plane_tri_vnt.obj", my_shader));

    Model* teapotModel = new Model("assets/obj/teapot_tri_vnt.obj", my_shader);
    GLuint tex = textureInit("assets/box.png");
    teapotModel->setTexture(tex);
    teapotModel->alpha = 0.5f;
    entities.emplace("teapot", new Entity(glm::vec3(20, 5, 5), teapotModel));
    entities.emplace("teapot2", new Entity(glm::vec3(30, 5, 5), teapotModel));


    Model* cameraModel = new Model("assets/obj/minecraft_simple_rig.obj", my_shader);
    GLuint tex1 = textureInit("assets/textures/Char.png");
    camera.addModel(cameraModel);
    cameraModel->setTexture(tex1);
    entities.insert(std::make_pair("camera", &camera));

    Model* FishModel = new Model("assets/obj/fish.obj", my_shader);
    Entity* fish = new Entity(glm::vec3(5, 5, 5), FishModel);
    entities.emplace("fish", fish); // passes a pointer


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

        debug = false;
        ShaderProgram debug_shader("assets/shaders/debug.vert",
            "assets/shaders/debug.frag");
        ShaderProgram particle_shader("assets/shaders/particles.vert",
            "assets/shaders/particles.frag");
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);


        DirectionalLight* sun = new DirectionalLight();
        SpotLight* flashlight = new SpotLight();
        flashlight->color = glm::vec3(0.2f, 0.2f, 1.0f);
        flashlight->cutOff = glm::cos(glm::radians(12.5f));
        flashlight->outerCutOff = glm::cos(glm::radians(17.5f));
		AmbientLight* ambient = new AmbientLight();
		ambient->color = glm::vec3(0.5f, 0.5f, 0.5f);
		PointLight* pointLight = new PointLight();
        lights.push_back(ambient);
        lights.push_back(sun);
        lights.push_back(flashlight);

        entities["teapot"]->addBehavior(Behaviors::WalkInCircle(glm::vec3(10, 0, 10), 5.0f, 10.0f));
        entities["teapot"]->addBehavior(Behaviors::Spin());
        entities["teapot"]->addBehavior(Behaviors::PeriodicJump(7.0f, 3.0f));


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

            camera.processKeyboard(pressedKeys, deltaTime);
            for (auto& [name, entity] : entities) {
                float height = height_map.getHeightAt(entity->position);
                entity->update(deltaTime, height);
                
            }
            flashlight->position = camera.position;
            flashlight->direction = camera.front;

            float sunAngle = currentFrame * 2.0f; // radians per second
            sun->direction = glm::normalize(glm::vec3(cos(sunAngle), -1.0f, sin(sunAngle)));
			pointLight->position = camera.position;

            Particles::update(deltaTime);

            // Collisions
            for (auto& [nameA, a] : entities) {
                for (auto& [nameB, b] : entities) {
                    if (a == b) continue;

                    float dist = glm::distance(a->position, b->position);
                    float combinedRadius = a->getCollisionRadius() + b->getCollisionRadius();

                    if (dist < combinedRadius) {
                        glm::vec3 aMin = a->position + a->model->boundingBoxMin;
                        glm::vec3 aMax = a->position + a->model->boundingBoxMax;
                        glm::vec3 bMin = b->position + b->model->boundingBoxMin;
                        glm::vec3 bMax = b->position + b->model->boundingBoxMax;

                        bool intersects =
                            (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
                            (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
                            (aMin.z <= bMax.z && aMax.z >= bMin.z);

                        if (!intersects) continue;

                        // Normalize direction from B to A
                        glm::vec3 dir = glm::normalize(a->position - b->position);

                        // Push each entity away from the other by half the overlap
                        float overlap = combinedRadius - dist;
                        glm::vec3 correction = dir * (overlap * 0.5f);

                        //a->position += correction;
                        //b->position -= correction;

                        // Optional: bounce a bit (exchange momentum or apply force)
                        a->velocity += dir * 10.0f; // tweak strength as needed
                        b->velocity -= dir * 10.0f;

                        // Get bounding box world-space centers
                        glm::vec3 centerA = a->position + (a->model->boundingBoxMin + a->model->boundingBoxMax) * 0.5f;
                        glm::vec3 centerB = b->position + (b->model->boundingBoxMin + b->model->boundingBoxMax) * 0.5f;

                        // Midpoint between bounding box centers
                        glm::vec3 impactPoint = (centerA + centerB) * 0.5f;

                        // Spawn particles at the impact point
                        Particles::spawn(impactPoint, 100);
                    }
                }
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
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
            glm::mat4 view = camera.getViewMatrix(); // Get updated camera view

            // Set transformation matrix (uMVP) for the model
            glm::mat4 uMVP = projection * view;

            GLint mvpLoc = glGetUniformLocation(shader_prog_ID, "uMVP");
            if (mvpLoc != -1) {
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(uMVP));
            }
            else {
                std::cerr << "ERROR: uMVP uniform not found in shader!" << std::endl;
            }

            height_map.draw(projection, view, lights);
            if (debug) {
                for (auto& [name, entity] : entities) {
                    entity->drawBoundingBox(projection, view, debug_shader);
                }
            }

            // Iterate through the scene and render each model using `model.draw()`
            for (auto& [name, model] : scene) {
                model.draw(projection, view, lights);
            }

            Particles::drawParticles(projection, view, debug_shader);

            transparent.clear();
            // Render Dynamic Entities (Entities)
            for (auto& [name, entity] : entities) {
                //entity.jump(10);
                //entity.rotate(5, 0);
                if (entity->model->alpha == 1) {
                    entity->render(shader_prog_ID, projection, view, lights);
                }
                else {
                    transparent.push_back(entity);
                }
            }

            std::sort(transparent.begin(), transparent.end(), [&](Entity const* a, Entity const* b) {
                return glm::distance(camera.getEfPos(), a->position) > glm::distance(camera.getEfPos(), b->position);
                });

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            for (Entity* entity : transparent) {
                entity->render(shader_prog_ID, projection, view, lights);
            }
            glDepthMask(GL_TRUE); // re-enable depth writes

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
        case GLFW_KEY_N: {
            samples = this_inst->loadAASamplesFromConfig("settings.json");
            switch (samples) {
            case 1: samples = 2; break;
            case 2: samples = 4; break;
            case 4: samples = 8; break;
            case 8: samples = 1; break;
            }

            // Save the new value to settings.json
            std::ifstream inFile("settings.json");
            nlohmann::json config;

            if (inFile.is_open()) {
                inFile >> config;
                inFile.close();
            }

            config["antialiasing_samples"] = samples;

            std::ofstream outFile("settings.json");
            if (outFile.is_open()) {
                outFile << config.dump(4); // pretty print with 4-space indentation
                outFile.close();
                std::cout << "Switching AA to: " << samples << "x (will apply on restart)\n";
            }
            else {
                std::cerr << "Error: Could not save settings.json\n";
            }

            break;
        }

        case GLFW_KEY_M:
            if (this_inst->debug) {
                this_inst->debug = false;
            }
            else{
                this_inst->debug = true;
                std::cerr << "Box debug on"<< std::endl;
            }
            break;
        case GLFW_KEY_L:
        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            if (!this_inst->fullscreen) {
                // Save windowed mode size/position
                glfwGetWindowPos(window, &this_inst->windowPosX, &this_inst->windowPosY);
                glfwGetWindowSize(window, &this_inst->windowWidth, &this_inst->windowHeight);

                // Switch to fullscreen
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                this_inst->fullscreen = true;
                std::cout << "Switched to fullscreen\n";
            }
            else {
                // Return to saved windowed state
                glfwSetWindowMonitor(window, nullptr,
                    this_inst->windowPosX,
                    this_inst->windowPosY,
                    this_inst->windowWidth,
                    this_inst->windowHeight,
                    0);
                this_inst->fullscreen = false;
                std::cout << "Switched to windowed mode\n";
            }
            break;
        }

        default:
            break;
        }
    }
}

int App::loadAASamplesFromConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file, using default AA = 1" << std::endl;
        return 1; // Fallback
    }

    nlohmann::json config;
    file >> config;

    if (config.contains("antialiasing_samples") && config["antialiasing_samples"].is_number_integer()) {
        std::cerr << "Loaded config" << std::endl;
        return config["antialiasing_samples"];
    }

    std::cerr << "Warning: Invalid or missing 'antialiasing_samples' in config" << std::endl;
    return 1;
}
