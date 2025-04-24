#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

class SettingManager {
public:
	int windowWidth;
	int windowHeight;
	int windowPosX;
	int windowPosY;
	bool fullscreen;
	bool vsync_on;
	int antialiasing_samples;


	SettingManager(const std::string& filename) {
		// Load settings from the JSON file
		loadSettings(filename);
	}

	void loadSettings(const std::string& filename) {
		std::ifstream inFile(filename);
		if (inFile.is_open()) {
			nlohmann::json config;
			inFile >> config;
			inFile.close();
			windowWidth = config["windowWidth"];
			windowHeight = config["windowHeight"];
			windowPosX = config["windowPosX"];
			windowPosY = config["windowPosY"];
			fullscreen = config["fullscreen"];
			vsync_on = config["vsync_on"];
			antialiasing_samples = config["antialiasing_samples"]; // Default to no AA
		}
		else {
			std::cerr << "Error: Could not open settings file: " << filename << std::endl;
		}
	}
	void saveSettings(const std::string& filename) {
		std::ofstream outFile(filename);
		if (outFile.is_open()) {
			nlohmann::json config;
			config["windowWidth"] = windowWidth;
			config["windowHeight"] = windowHeight;
			config["windowPosX"] = windowPosX;
			config["windowPosY"] = windowPosY;
			config["fullscreen"] = fullscreen;
			config["vsync_on"] = vsync_on;
			config["antialiasing_samples"] = antialiasing_samples;
			outFile << config.dump(4); // Pretty print with 4-space indentation
			outFile.close();
		}
		else {
			std::cerr << "Error: Could not save settings to file: " << filename << std::endl;
		}
	}
};