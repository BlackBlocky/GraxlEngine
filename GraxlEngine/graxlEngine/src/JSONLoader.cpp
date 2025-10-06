#include "graxlEngine/JSONLoader.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace nlohmann;

namespace graxl {

Scene JSONLoader::loadScene(std::string path)
{
	// Load JSON Data and create basic Scene
	std::fstream file(path);
	if (file.fail()) {
		spdlog::critical("JSONLoader: Error opening JSON-Scene file: {}", path);
		throw std::runtime_error("Error opening JSON-Scene file: " + path);
	}

	json jsonData = json::parse(file);

	std::string sceneName = jsonData["sceneName"].get<std::string>();
	Scene newScene(sceneName, "");

	// Load UIElements from JSON and add it to the scene
	std::vector<json> uiElements = jsonData["ui"].get<std::vector<json>>();
	for (json& j : uiElements) {
		newScene.uiElements.push_back(
			UIElement(
				UIElement::UIElementType::Label,
				Vector2{ (float)j["posX"].get<float>(), (float)j["posY"].get<float>() },
				Vector2{ (float)j["width"].get<float>(), (float)j["height"].get<float>() },
				j["id"].get<std::string>(),
				j["textValue"].get<std::string>(),
				j["boolValue"].get<bool>()
			)
		);
	}

	return newScene;
}

void JSONLoader::loadSettings()
{

}

};
