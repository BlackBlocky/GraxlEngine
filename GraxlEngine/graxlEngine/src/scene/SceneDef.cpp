#include <graxlEngine/scene/SceneDef.h>

#include <spdlog/spdlog.h>

namespace graxl {

	Scene SceneDef::build()
	{
		spdlog::trace("SceneDef: Building scene {} with id {}", sceneName, sceneId);

		Scene newScene = Scene(sceneName, sceneId);

		if (_buildFunc != nullptr) {
			spdlog::trace("SceneDef: Building {} with the build function.", sceneId);
			_buildFunc(newScene);
		}
		else {
			spdlog::trace("SceneDef: Build fucntion is nullptr for {}, no build function will be executed", sceneId);
		}

		return newScene;
	}

}