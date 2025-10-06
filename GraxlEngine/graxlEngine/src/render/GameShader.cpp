#include <graxlEngine/render/GameShader.h>

#include <filesystem>

#include <spdlog/spdlog.h>

namespace graxl {

	GameShader::~GameShader() {
		if (_movedFrom) return;

		if (!_shaderWasLoaded) return; // Thats means a shader was ever loaded, e.g. it could be a non GLSL shader

		if (IsShaderValid(_myShader.value())) {
			// If we get here, that means the shader wasnt unloaded - MEMORYYY LEAKKKK
			spdlog::warn("GameShader: POSSIBLE MEMORY LEAK - GameShader Object \"{}\" being destroyed, but the glsl shader was never unloaded!", c_name);
			spdlog::warn("...unloading {} in decontructur", c_name);
			unloadGameShader();

			//assert(false && "GamsShader being destroyed, but shader wasnt unloaded!");
		}
	}

	int GameShader::loadGLSLShaderFromFile(std::string path)
	{
		if (c_type != GameShaderType::GLSL) {
			spdlog::error("GameShader: Tried to load a GLSL shader on a non GLSL shader typed GameShader: {}", c_name);
			return 3; // Wrong type error code
		}
		if (!std::filesystem::exists(path)) {
			spdlog::error("GameShader: File {} not found for GameShader: {}", path, c_name);
			return 2; // Error code - File doesnt Exist
		}		
	
		spdlog::debug("GameShader: Loading shader \"{}\" at {}", c_name, path);
		_myShader = LoadShader(NULL, path.c_str());
		_shaderWasLoaded = true;
		return 0;
	}

	void GameShader::unloadGameShader()
	{
		if (_movedFrom) {
			spdlog::warn("GameShader: Tried to unload shader \"{}\", but this object was moved from!", c_name);
			return;
		}

		if (!_shaderWasLoaded) return; // Dont need to unload anything
		if (!IsShaderValid(_myShader.value())) {
			spdlog::warn("GameShader: Tried to unload shader \"{}\", but it is already unloaded!", c_name);
			return;
		}

		spdlog::debug("GameShader: Unloading shader \"{}\"", c_name);
		UnloadShader(_myShader.value());
		_myShader.value().id = 0;
	}

	const std::optional<Shader>& GameShader::getShader() const
	{
		return (_shaderWasLoaded && _myShader) ? _myShader : std::nullopt;
	}

}