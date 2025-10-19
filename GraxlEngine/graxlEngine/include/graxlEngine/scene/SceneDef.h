#pragma once

#include <functional>
#include <string>
#include <memory>

#include <graxlEngine/scene/Scene.h>

namespace graxl {

	class SceneDef {
	public:
		SceneDef(const std::string& id, const std::string& name, std::function<void(Scene& scene)> buildFunction)
			: sceneId(id), sceneName(name), _buildFunc(buildFunction) {
		};

		SceneDef() {}; // Default constructor so that std::unordered_map doesn't throw compile errors lol

		// Creating a new scene and running the _buildFunc with the new scene as param. After its done, it returns the result
		Scene build();

		const std::string sceneId;
		const std::string sceneName;

		bool haveBuildFunction() const {
			return _buildFunc != nullptr;
		}

	private:
		std::function<void(Scene& scene)> _buildFunc;

	};

}