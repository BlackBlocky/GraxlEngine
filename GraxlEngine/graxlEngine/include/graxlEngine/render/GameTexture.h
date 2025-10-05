#pragma once

#include <raylib.h>
#include <spdlog/spdlog.h>

#include <string>

namespace graxl {

// IMPORTANT: Only References should be passed to that object, DO NOT MAKE A COPY, otherwise GPU goes boom because of texture unloading etc
class GameTexture {
public:
	// The Texture2D have to be direclty initialized when initalizing this Object. Only this Object should be the owner of the Texture2D.
	GameTexture(Texture2D&& texture, std::string textureName) 
		: _myTexture(std::move(texture)), myTextureRect(Rectangle{ 0.0f, 0.0f, (float)texture.width, (float)texture.height}), name(textureName), _isLoaded(true) {
		spdlog::trace("GameTexture: Created GameTexture \"{}\" ({}x{})", name, texture.width, texture.height);
	};
	~GameTexture() {
		if (IsTextureValid(_myTexture)) {
			spdlog::warn("GameTexture: POSSIBLE MEMORY LEAK - GameTexture is being destroyed, but Texture \"{}\" is still loaded into VRAM", name);
			spdlog::warn("...unloading {} in decontructur", name);
			unloadGameTexture();

			//assert(false && "GameTexture destroyed without unloading – possible resource leak!"); // Debug error
		}	
	};

	// Denie Copy-Costructors
	GameTexture(const GameTexture&) = delete;
	GameTexture& operator=(const GameTexture&) = delete;

	// Move Constructor
	GameTexture(GameTexture&& other) noexcept
		: _myTexture(std::move(other._myTexture)), myTextureRect(other.myTextureRect), name(other.name), _isLoaded(true)
	{
		other._myTexture.id = 0;
		other._isLoaded = false;
	}

	// Unloads all textures related to this GameTexture from VRAM
	void unloadGameTexture()
	{
		if (IsTextureValid(_myTexture)) {
			spdlog::debug("GameTexture: Unloading Texture \"{}\" from VRAM...", name);
			UnloadTexture(_myTexture);
			_isLoaded = false;
			_myTexture.id = 0;
		}
		else {
			spdlog::warn("GameTexture: Tried to unload Texture \"{}\", but its already unloaded!", name);
		}
	}

	const Texture2D& getTexture() const { return _myTexture; };
	const Rectangle myTextureRect;
	const std::string name;

	const bool isLoaded() const { return _isLoaded; }

private:
	Texture2D _myTexture; // Not const so we can do unload actions and stuff
	bool _isLoaded;
};

}