#include <graxlEngine/render/GameShader.h>

#include <filesystem>

#include <spdlog/spdlog.h>
#include <magic_enum/magic_enum.hpp>

namespace graxl {

	GameShader::GameShader(const std::string shaderName, const GameShaderType shaderType, const int __shaderPriority, const int __customValuesAmount)
		: c_name(shaderName), _shaderWasLoaded(false), c_type(shaderType), c_shaderPriority(__shaderPriority),
		_movedFrom(false), c_customValuesAmount(__customValuesAmount) {
		spdlog::trace("GameShader: Created GameShader \"{}\" of type {} with priority {} and customValuesAmount {}", c_name, magic_enum::enum_name(c_type), c_shaderPriority, c_customValuesAmount);

		if (shaderType == GameShaderType::GLSL) {
			spdlog::trace("GameShader: ... Creating shaderUniformData for {}, because its type is GLSL", shaderName);
			_shaderUniformData = std::make_shared<ShaderUniformData>();
			_shaderUniformData->globalTime = 0.0f;
			_shaderUniformData->songTime = 0.0f;
			_shaderUniformData->intensity = 0.0f;
			_shaderUniformData->customValues.resize(c_customValuesAmount, 0.0f);
		}
	}

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

	int GameShader::loadGLSLShaderFromFile(const std::filesystem::path& path)
	{
		if (c_type != GameShaderType::GLSL) {
			spdlog::error("GameShader: Tried to load a GLSL shader on a non GLSL shader typed GameShader: {}", c_name);
			return 3; // Wrong type error code
		}
		if (!std::filesystem::exists(path)) {
			spdlog::error("GameShader: File {} not found for GameShader: {}", path.string(), c_name);
			return 2; // Error code - File doesnt Exist
		}		
	
		spdlog::debug("GameShader: Loading shader \"{}\" at {}", c_name, path.string());
		_myShader.emplace(LoadShader(NULL, path.string().c_str()));
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

	const std::optional<Shader>* GameShader::getShader() const
	{
		return (_shaderWasLoaded && _myShader) ? &_myShader : nullptr;
	}

}