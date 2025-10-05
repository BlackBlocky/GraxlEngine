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
	auto it = std::find_if(shaders.begin(), shaders.end(), [&](const auto& ifShader) {
		return shader.getPriority() < ifShader->getPriority();
	});

	// Move the shader with the higher priority to the right, and insert the new shader at its position
	shaders.insert(it, std::make_shared<GameShader>(std::move(shader)));
}

void Scene::unloadScene()
{
	spdlog::info("Scene: Unloading scene {}...", sceneName);
	for (auto& shader : shaders) {
		shader->unloadGameShader();
	}
	shaders.clear();
	spdlog::info("Scene: Scene {} unloaded.", sceneName);
}

}