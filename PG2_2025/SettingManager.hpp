#pragma once



struct settings {
	int windowWidth = 1280;
	int windowHeight = 720;
	int windowPosX = 100;
	int windowPosY = 100;
	bool fullscreen = false;
	bool vsync_on = true;
	int antialiasing_samples = 1; // Default to no AA
};

class SettingManager {
public:
	settings appSettings;
	SettingManager() {
		// Load settings from JSON file
		loadSettings("settings.json");
	}
	void loadSettings(const std::string& filename) {
		std::ifstream inFile(filename);
		if (inFile.is_open()) {
			nlohmann::json config;
			inFile >> config;
			inFile.close();
			appSettings.windowWidth = config.value("windowWidth", 1280);
			appSettings.windowHeight = config.value("windowHeight", 720);
			appSettings.windowPosX = config.value("windowPosX", 100);
			appSettings.windowPosY = config.value("windowPosY", 100);
			appSettings.fullscreen = config.value("fullscreen", false);
			appSettings.vsync_on = config.value("vsync_on", true);
			appSettings.antialiasing_samples = config.value("antialiasing_samples", 1); // Default to no AA
		}
		else {
			std::cerr << "Error: Could not open settings file: " << filename << std::endl;
		}
	}
	void saveSettings(const std::string& filename) {
		std::ofstream outFile(filename);
		if (outFile.is_open()) {
			nlohmann::json config;
			config["windowWidth"] = appSettings.windowWidth;
			config["windowHeight"] = appSettings.windowHeight;
			config["windowPosX"] = appSettings.windowPosX;
			config["windowPosY"] = appSettings.windowPosY;
			config["fullscreen"] = appSettings.fullscreen;
			config["vsync_on"] = appSettings.vsync_on;
			config["antialiasing_samples"] = appSettings.antialiasing_samples;
			outFile << config.dump(4); // Pretty print with 4-space indentation
			outFile.close();
		}
		else {
			std::cerr << "Error: Could not save settings to file: " << filename << std::endl;
		}
	}
};