#pragma once

#include "graxlEngine/ui/UIElement.h"
#include "graxlEngine/scene/GameObject.h"
#include "graxlEngine/render/GameShader.h"

#include <string>
#include <vector>
#include <memory>

namespace graxl {

class Scene {
public:
	Scene(const std::string& name) : sceneName(name) {}

	const std::string sceneName;

	std::vector<UIElement> uiElements;
	std::vector<std::unique_ptr<GameObject>> gameObjects; // With pointer for potential polymorphism in the future
	
	const std::vector<std::shared_ptr<GameShader>>& getShaders() const { return shaders; }
	void addShader(GameShader&& shader); // Will overtake the shaders ownership, and moving the shader intos the scenes "RAM". Also sorting the shader array by priority
		
	// Unload all resources, that are connected to this scene, like shaders
	void unloadScene();

private:
	std::vector<std::shared_ptr<GameShader>> shaders;
};

};