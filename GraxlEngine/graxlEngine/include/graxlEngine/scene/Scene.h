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
	Scene(const std::string& name, const std::string& __sceneId) : sceneName(name), sceneId(__sceneId) {}

	const std::string sceneName;
	// Used for scene regestry, should be unique
	const std::string sceneId;

	std::vector<UIElement> uiElements;
	
	const std::vector<std::shared_ptr<GameShader>>& getShaders() const { return _shaders; } // (note for me: wont affect ownership, just a reference)
	// Will overtake the shaders ownership, and moving the shader intos the scenes "RAM". Also sorting the shader array by priority
	void addShader(GameShader&& shader);
		
	std::weak_ptr<GameObject> getGameObjectByName(const std::string& name);
	const std::vector<std::weak_ptr<GameObject>> getGameObjectsByName(const std::string& name);
	// Will move the gameObject into the scenes "RAM", aka taking ownership
	void addGameObject(GameObject&& gameObject);
	void destroyGameObject(std::weak_ptr<GameObject>& gameObject);
	const std::vector<std::shared_ptr<GameObject>>& getAllGameObjects() { return _gameObjects; };
	const std::vector<std::weak_ptr<const GameObject>> getAllGameObjects() const;

	// Unload all resources, that are connected to this scene, like shaders
	void unloadScene();

private:
	std::vector<std::shared_ptr<GameShader>> _shaders;
	std::vector<std::shared_ptr<GameObject>> _gameObjects; // With pointer for potential polymorphism in the future
};

};