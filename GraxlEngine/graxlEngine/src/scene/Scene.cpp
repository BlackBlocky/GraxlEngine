#include <graxlEngine/scene/Scene.h>

#include <spdlog/spdlog.h>
#include <algorithm>
#include <exception>

#include <magic_enum/magic_enum.hpp>

namespace graxl {
	const std::weak_ptr<GameTexture> Scene::getTexture(const std::string& key)
	{
		if (!_texturesMap.contains(key)) {
			spdlog::error("Scene: Tried to get texture with key {} in scene {}. But the scene does not have a texture with that key!", key, this->sceneName);
			return std::weak_ptr<GameTexture>();
		}

		spdlog::trace("Scene: Returned weakptr for GameTexture {}", key);
		return std::weak_ptr<GameTexture>(_texturesMap[key]);
	}

	std::weak_ptr<GameTexture> Scene::loadTexture(const std::string& key, const std::filesystem::path& path, const std::string& name)
	{
		if (_texturesMap.contains(key)) {
			spdlog::error("Scene: Tried to load texture into the scene \"{}\" with an already used key: \"{}\"", this->sceneName, key);
			return std::weak_ptr<GameTexture>();
		}

		spdlog::trace("Scene: Loading texture with key {} to scene {}", key, this->sceneName);
		_texturesMap.emplace(key, std::make_shared<GameTexture>(path, name));
		return std::weak_ptr<GameTexture>(_texturesMap[key]);
	}

	// Only use this funtion specifically needed. For simply loading textures, you should use .loadTexture(...)
	std::weak_ptr<GameTexture> Scene::addTextureObject(const std::string& key, GameTexture&& texture)
	{
		if (_texturesMap.contains(key)) {
			spdlog::error("Scene: Tried to add textureObject into the scene \"{}\" with an already used key: \"{}\"", this->sceneName, key);
			return std::weak_ptr<GameTexture>();
		}

		spdlog::trace("Scene: Adding texture with key {} to scene {}", key, this->sceneName);
		_texturesMap.emplace(key, std::make_shared<GameTexture>(std::move(texture)));
		return std::weak_ptr<GameTexture>(_texturesMap[key]);
	}

	std::weak_ptr<GameShader> Scene::createSimpleShader(const std::string& key, const std::string& shaderName, GameShaderType shaderType)
	{
		// Validate
		if (shaderType == GameShaderType::GLSL) throw std::invalid_argument("GameShaderType GLSL is not allowed in createSimpleShader(). Use createGlslShader() instead.");
		if (_shadersMap.contains(key)) {
			spdlog::error("Scene: Tired to add Simple shader \"{}\" with key \"{}\", the the key is already used!", shaderName, key);
			return std::weak_ptr<GameShader>();
		}

		// Create Shader
		spdlog::trace("Scene: Creating simple shader \"{}\" with GameShaderType of \"{}\"", shaderName, magic_enum::enum_name(shaderType));
		_shadersMap.emplace(key, std::make_shared<GameShader>(shaderName, shaderType, INT32_MIN, 0));

		_resortSortedShaderList();

		return _shadersMap[key];
	}

	std::weak_ptr<GameShader> Scene::createGlslShader(const std::string& key, const std::string& shaderName, const std::filesystem::path& path, int priority, int customValuesAmount)
	{
		// Validate
		if (_shadersMap.contains(key)) {
			spdlog::error("Scene: Tired to add GLSL shader \"{}\" with key \"{}\", the the key is already used!", shaderName, key);
			return std::weak_ptr<GameShader>();
		}
		if (!std::filesystem::exists(path)) {
			spdlog::error("Scene: Tried to load GLSL Shader \"{}\", but filepath \"{}\" doesnt exist!", shaderName, path.string());
			return std::weak_ptr<GameShader>();
		}

		// Create Shader
		spdlog::trace("Scene: Loading GLSL Shader \"{}\" on with key \"{}\" on path \"{}\"", shaderName, key, path.string());
		_shadersMap.emplace(key, std::make_shared<GameShader>(shaderName, GameShaderType::GLSL, priority, customValuesAmount));
		_shadersMap[key]->loadGLSLShaderFromFile(path);

		_resortSortedShaderList();

		return _shadersMap[key];
	}

	std::weak_ptr<GameShader> Scene::getShaderByKey(const std::string& key)
	{
		if (!_shadersMap.contains(key)) {
			spdlog::warn("Scene: Tried to get shader \"{}\" in scene \"{}\", but there is no shader with that key!", key, sceneName);
			return std::weak_ptr<GameShader>();
		}

		spdlog::trace("Scene: Getter for Shader found \"{}\" in scene \"{}\". Returning weak_ptr...", key, sceneName);
		return _shadersMap[key];
	}

	std::weak_ptr<GameObject> Scene::getGameObjectByName(const std::string& name)
	{
		auto it = std::find_if(_gameObjects.begin(), _gameObjects.end(), [&name](const auto& ifGo) {
			return ifGo->name == name;
		});

		if (it != _gameObjects.end()) {
			return std::weak_ptr<GameObject>(*it);
		}
		else return std::weak_ptr<GameObject>(); // return empy weak_ptr (null)
	}

	const std::vector<std::weak_ptr<GameObject>> Scene::getGameObjectsByName(const std::string& name)
	{
		std::vector<std::weak_ptr<GameObject>> foundGameObjects;

		for (auto& go : _gameObjects) {
			if (go->name == name) foundGameObjects.push_back(std::weak_ptr<GameObject>(go));
		}

		return foundGameObjects;
	}

	std::weak_ptr<GameObject> Scene::createGameObject(const std::string& name, const std::string& textureKey)
	{
		spdlog::trace("Scene: Creating GameObject {} in scene {} with textureKey {}", name, this->sceneName, textureKey);
		_gameObjects.emplace_back(std::make_shared<GameObject>(this->getTexture(textureKey), name));

		return std::weak_ptr<GameObject>(_gameObjects.back());
	}

	void Scene::destroyGameObject(std::weak_ptr<GameObject>& gameObject)
	{
		auto gameObjectPtr = gameObject.lock();

		// Check if the pointer is valid
		if (gameObjectPtr == nullptr) {
			spdlog::warn("Scene: Tried to destroy a GameObject in scene {}, but the weak_ptr was expired!", sceneName);
			return;
		}

		// Search the _gameObjects vector, if it contains a gameObject that is on the same adress as the given gameObject
		auto foundIt = std::find_if(_gameObjects.begin(), _gameObjects.end(), [&gameObjectPtr](const auto& ifGo) {
			return ifGo == gameObjectPtr;
		});

		// Check if we found it
		if (foundIt == _gameObjects.end()) {
			spdlog::warn("Scene: Tried to destroy GameObject {} in Scene {}, but the scene doesnt contain it. (SceneId:{})",
				gameObjectPtr->name, sceneName, sceneId);
		}

		// Remove it from the vector
		_gameObjectDestroyProtocol(*foundIt->get());
		_gameObjects.erase(foundIt);
	}

	const std::vector<std::weak_ptr<const GameObject>> Scene::getAllGameObjectsConst() const
	{
		std::vector<std::weak_ptr<const GameObject>> returnResult;

		returnResult.reserve(_gameObjects.size());
		for (const auto& goSharedPtr : _gameObjects) {
			returnResult.push_back(goSharedPtr);
		}

		return returnResult;
	}

	void Scene::unloadScene()
	{
		spdlog::info("Scene: Unloading scene {}...", sceneName);
		for (auto& shader : _shadersMap) {
			shader.second->unloadGameShader();
		}
		_shadersMap.clear();

		for (auto& texture : _texturesMap) {
			texture.second->unloadGameTexture();
		}
		_texturesMap.clear();

		// NOTE: During Scene unload, no destroyGameObject() calls are made.
		//       We directly clear _gameObjects here.
		for (auto& go : _gameObjects) {
			_gameObjectDestroyProtocol(*go);
			go.reset();
		}
		_gameObjects.clear();

		spdlog::info("Scene: ... Scene {} unloaded.", sceneName);
	}

	void Scene::_resortSortedShaderList()
	{
		_sortedShaderList.clear();

		// Move every Shader into the list
		for (auto& shaderSharedPtr : _shadersMap) {
			_sortedShaderList.push_back(shaderSharedPtr.second);
		}

		// Sort the list
		std::sort(_sortedShaderList.begin(), _sortedShaderList.end(), [](std::weak_ptr<GameShader>& sortShaderA, std::weak_ptr<GameShader>& sortShaderB){
			return sortShaderA.lock()->getPriority() < sortShaderB.lock()->getPriority();
		});
	}

	void Scene::_gameObjectDestroyProtocol(GameObject& target)
	{
		spdlog::trace("Scene: Destroying GameObject\"{}\"...");

		// No function yet, but already here, well yeah
	}

}