#pragma once

#include <string>
#include <optional>
#include <vector>

#include <raylib.h>
#include <spdlog/spdlog.h>

namespace graxl {

// If the type is not GLSL, the shader is not an actual shader, it just modifies the camera.
// Non GLSL Shaders are directly implemented into the renderer
const enum class GameShaderType {
	ROTATION, ZOOM, SCREENSHAKE, GLSL
};

// The GameShader should be the ONLY object, that uses a specific shader. Which means, DO NOT COPY IT, otherwise GPU may goes boom. And always onload if you need to
class GameShader {
public:
	// Constructor / Deconstructur
	GameShader(const std::string shaderName, const GameShaderType shaderType, const int __shaderPriority, const int __customValuesAmount)
		: c_name(shaderName), _shaderWasLoaded(false), c_type(shaderType), c_shaderPriority(__shaderPriority),
		_movedFrom(false), c_customValuesAmount(__customValuesAmount) { 
        spdlog::trace("GameShader: Created GameShader \"{}\" of type {} with priority {} and customValuesAmount {}", c_name, (int)c_type, c_shaderPriority, c_customValuesAmount);
	}
	~GameShader();

	// Copy Constructor
	GameShader(const GameShader&) = delete;
	GameShader& operator=(const GameShader&) = delete;

	// Move Constructor
	GameShader(GameShader&& other) noexcept
		: c_name(other.c_name), _shaderWasLoaded(other._shaderWasLoaded), c_type(other.c_type), _myShader(std::exchange(other._myShader, std::nullopt)),
		c_shaderPriority(other.c_shaderPriority), c_customValuesAmount(other.c_customValuesAmount), _shaderUniformData(std::move(other._shaderUniformData))
	{ 
		other._movedFrom = true;
		this->_movedFrom = false;
	}
	GameShader& operator=(GameShader&& other) noexcept
	{
		if (this != &other) {
			c_name = other.c_name;
			_shaderWasLoaded = other._shaderWasLoaded;
			c_type = other.c_type;
			_myShader = std::exchange(other._myShader, std::nullopt);
			c_shaderPriority = other.c_shaderPriority;
			c_customValuesAmount = other.c_customValuesAmount;
			_shaderUniformData = std::move(other._shaderUniformData);

			other._movedFrom = true;
			this->_movedFrom = false;
		}
		return *this;
	}


	// Containg all data, that actively needs to be updated
	struct ShaderUniformData {
		float globalTime; // Time since the itself game started (Raylibs time)
		float songTime; // Time since the song started playing

		float intensity; // Intented to be a value between 0 and 1
		std::vector<float> customValues; // Size is specified by customValuesAmount
	};


	// Methods etc.

	// Returns: 0=Succes, 1=UnkownError, 2=FileNotFound, 3=NoAllowed
	int loadGLSLShaderFromFile(std::string path);
	void unloadGameShader();

	const std::optional<Shader>& getShader() const;
	bool isShaderLoaded() const { return _shaderWasLoaded && _myShader && IsShaderValid(_myShader.value()); }

	const std::string& getName() const { return c_name; }
	const GameShaderType getType() const { return c_type; }
	const int getPriority() const { return c_shaderPriority; }
	const int getCustomValuesAmount() const { return c_customValuesAmount; }

	// Returns a shared pointer to the ShaderUniformData. If it doesnt exist yet, it will be created and initialized with default values.
	// Should be used for modifying the shader parameters
	std::shared_ptr<ShaderUniformData> getShaderUniformDataPtr() {
		if (_movedFrom) {
			spdlog::warn("GameShader: Tried to get ShaderUniformData of shader \"{}\", but this object was moved from! Returning nullptr.", c_name);
			return nullptr;
		}

		if (!_shaderUniformData) {
			_shaderUniformData = std::make_shared<ShaderUniformData>();
			_shaderUniformData->globalTime = 0.0f;
			_shaderUniformData->songTime = 0.0f;
			_shaderUniformData->intensity = 0.0f;
			_shaderUniformData->customValues.resize(c_customValuesAmount, 0.0f);
		}
		return _shaderUniformData;
	}
	// Returns a const reference. Read-only
	const ShaderUniformData& getShaderUniformData() const {
		if (_movedFrom) {
			spdlog::warn("GameShader: Tried to get const ShaderUniformData of shader \"{}\", but this object was moved from! Returning a default value.", c_name);
			static ShaderUniformData defaultData = { 0.0f, 0.0f, 0.0f, std::vector<float>(c_customValuesAmount, 0.0f) };
			return defaultData;
		}

		if (!_shaderUniformData) {
			spdlog::warn("GameShader: Tried to get const ShaderUniformData of shader \"{}\", but it was never initialized! Returning a default value.", c_name);
			static ShaderUniformData defaultData = { 0.0f, 0.0f, 0.0f, std::vector<float>(c_customValuesAmount, 0.0f) };
			return defaultData;
		}
		return *_shaderUniformData;
	}
private:
	// Threat these 3 as const
	std::string c_name;
	GameShaderType c_type;
	int c_shaderPriority; // Higher = First
	int c_customValuesAmount;

	bool _shaderWasLoaded;
	std::optional<Shader> _myShader; // Always contains a glsl shader. Raylib shader.
	std::shared_ptr<ShaderUniformData> _shaderUniformData; // Contains all data that needs to be updated every frame

	bool _movedFrom;
};

}