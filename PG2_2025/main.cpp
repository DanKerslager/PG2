// main.cpp
#include <iostream> 
#include "app.hpp"
#include <chrono>
#include <filesystem>

// global App instance
App app;

int main()
{
    auto start = std::chrono::steady_clock::now();

    try {
        if (app.init(0)) {
            app.init_assets();
            app.init_hm();
            // Load texture before running the app
            GLuint mytex = app.textureInit("assets/box.png");  // <-- You need to have this file

            // Example: Assign to a model (you'd define the model inside App or elsewhere)
            // app.model.texture_id = mytex;

            // Or use app.setTextureForModel(mytex); if you create such a method

            return app.run();
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed: " << e.what() << std::endl;
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "elapsed time: " << elapsed_seconds.count() << " sec" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << " sec" << std::endl;

    return EXIT_SUCCESS;
}
