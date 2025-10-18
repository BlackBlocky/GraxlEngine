#pragma once

#include "graxlEngine/ui/UIElement.h"
#include "graxlEngine/scene/GameObject.h"
#include "graxlEngine/render/GameShader.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>

// A Scene is like a whole world. Every Shader, Texture, GameObjects. It all is here. If the Scene is being unloaded,
// everything will be gone aswell.

namespace graxl {

class Scene {
public:
	Scene(const std::string& name, const std::string& __sceneId) : sceneName(name), sceneId(__sceneId) {}

	const std::string sceneName;
	// Used for scene regestry, should be unique
	const std::string sceneId;

	std::vector<UIElement> uiElements;
	
	// --- TEXTURE ---
	const std::weak_ptr<GameTexture> getTexture(const std::string& key);
	// Loading a texture into the scene
	std::weak_ptr<GameTexture> loadTexture(const std::string& key, const std::filesystem::path& path, const std::string& name);
	// Will take the Textures ownership and moves it into the scene
	std::weak_ptr<GameTexture> addTextureObject(const std::string& key, GameTexture&& texture);

	// --- SHADER ---
	const std::vector<std::weak_ptr<GameShader>>& getShaderListSorted() const { return _sortedShaderList; };
	std::weak_ptr<GameShader> createSimpleShader(const std::string& key, const std::string& shaderName, GameShaderType shaderType);
	std::weak_ptr<GameShader> createGlslShader(const std::string& key, const std::string& shaderName, const std::filesystem::path& path, int priority, int customValuesAmount);
	std::weak_ptr<GameShader> getShaderByKey(const std::string& key);

	// --- GAME OBJECT ---
	std::weak_ptr<GameObject> getGameObjectByName(const std::string& name);
	const std::vector<std::weak_ptr<GameObject>> getGameObjectsByName(const std::string& name);
	// Creating an GameObject and loading it into the scene. The textureKey muste be defined in the scenes textures
	std::weak_ptr<GameObject> createGameObject(const std::string& name, const std::string& textureKey);
	// DO NOT call this in a for loop that iterates though all GameObjects. Use (not implemented yet :P) instead. This will push and event that will be processed next frame
	// In other words, do not call this inside a GameObject Component Function! (At least dont this is if you dont want a spectecular crash lol)
	// The function will instantly remove this GameObject from the _gameObjects vector
	void destroyGameObject(std::weak_ptr<GameObject>& gameObject);
	const std::vector<std::shared_ptr<GameObject>>& getAllGameObjects() { return _gameObjects; };
	const std::vector<std::weak_ptr<const GameObject>> getAllGameObjectsConst() const;

	// Unload all resources, that are connected to this scene, like shaders
	void unloadScene();

private:
	std::unordered_map<std::string, std::shared_ptr<GameShader>> _shadersMap; // The shaders are sorted by the priority
	std::vector<std::weak_ptr<GameShader>> _sortedShaderList; // From lowest to highest priority. Mainly used for rendering
	// Puts every GameShader in _shadersMap into _sortedShaderList sorted
	void _resortSortedShaderList();

	std::unordered_map<std::string, std::shared_ptr<GameTexture>> _texturesMap;
	std::vector<std::shared_ptr<GameObject>> _gameObjects; // With pointer for potential polymorphism in the future

	void _gameObjectDestroyProtocol(GameObject& target);
};

};