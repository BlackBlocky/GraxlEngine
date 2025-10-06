#include <graxlEngine/scene/Scene.h>

#include <spdlog/spdlog.h>
#include <algorithm>

namespace graxl {


// Will overtake the shaders ownership, and moving the shader intos the scenes "RAM"
// Please keep in mind that the source will be empty after the operation
void Scene::addShader(GameShader&& shader)
{
	spdlog::trace("Scene: Moving shader {} into scene {}...", shader.getName(), sceneName);

	// Insert-sort
	// Find the first shader with a higher priority than the new shader
	// Shaders with less priority are drawn first, so the higher priority shaders are drawn on top of the lower priority shaders
	auto it = std::find_if(_shaders.begin(), _shaders.end(), [&](const auto& ifShader) {
		return shader.getPriority() < ifShader->getPriority();
	});

	// Move the shader with the higher priority to the right, and insert the new shader at its position
	_shaders.insert(it, std::make_shared<GameShader>(std::move(shader)));
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

void Scene::addGameObject(GameObject&& gameObject)
{
	spdlog::trace("Scene: Moving GameObject {} into scene {}...", gameObject.name, sceneName);
	_gameObjects.push_back(std::make_shared<GameObject>(std::move(gameObject)));
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
	_gameObjects.erase(foundIt);
}

const std::vector<std::weak_ptr<const GameObject>> Scene::getAllGameObjects() const
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
	for (auto& shader : _shaders) {
		shader->unloadGameShader();
	}
	_shaders.clear();

	// Note: Textures from GameObjects wont be unloaded, because they only own a reference. (They can used across multiple GameObjects)

	spdlog::info("Scene: Scene {} unloaded.", sceneName);
}

}