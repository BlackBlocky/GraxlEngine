#pragma once

#include <raylib.h>
#include <spdlog/spdlog.h>

#include <string>

#include <filesystem>

namespace graxl {

// IMPORTANT: Only References should be passed to that object, DO NOT MAKE A COPY, otherwise GPU goes boom because of texture unloading etc
class GameTexture {
public:
	// Creating a GameTexture. Will load the texture from the given path. If the path is invalid, a placeholder texture will be created, with the size of 100x100
	GameTexture(const std::string texturePath, std::string textureName) : name(textureName) {
		
		// Check if the path to the texture is valid. If not, create a placeholder texture. Otherwise just load the texture normally
		if(!std::filesystem::exists(texturePath)) {
			spdlog::error("GameTexture: Texture path \"{}\" does not exist! Using Placeholder texture...", texturePath);
			// Creating a placeholder texture, beacuse the path is invalid. With the size of 100
			static Color placeholderPixels[100 * 100];
			static bool placeholderPixelsInitialized = false;
			if (!placeholderPixelsInitialized) {
				for (Color& c : placeholderPixels) c = MAGENTA;
				placeholderPixelsInitialized = true;
			}

			Image placeholderImage = { placeholderPixels, 100, 100, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
			_myTexture = LoadTextureFromImage(placeholderImage);
			// Note: We dont need to unload the image, because the pixel data is static and will be used for the whole program runtime

			_myTextureRect = Rectangle{ 0.0f, 0.0f, (float)_myTexture.width, (float)_myTexture.height };

			_isLoaded = true; // True, because we still need to unload the texture. Event if its a placeholder, the pixels still exist
		}
		else {
			// Just lopading the texture normally
			spdlog::trace("GameTexture: Loading Texture from path \"{}\"...", texturePath);
			_myTexture = LoadTexture(texturePath.c_str());
			_myTextureRect = Rectangle{ 0.0f, 0.0f, (float)_myTexture.width, (float)_myTexture.height };
			_isLoaded = true;
		}

		SetTextureFilter(_myTexture, TEXTURE_FILTER_POINT); // Setting a default filter mode

		// Done
		spdlog::trace("GameTexture: Created GameTexture \"{}\" ({}x{})", name, _myTexture.width, _myTexture.height);
	};
	~GameTexture() {
		// Just making sure the texture is unloaded before destroying the object
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
	GameTexture(GameTexture&& other) noexcept : name(other.name) {
		_myTexture = std::move(other._myTexture);
		_myTextureRect = std::move(other._myTextureRect);
		_isLoaded = other._isLoaded;

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
	const Rectangle& getTextureRect() const { return _myTextureRect; };
	const std::string name;

	const bool isLoaded() const { return _isLoaded; }

private:
	Texture2D _myTexture; // Not const so we can do unload actions and stuff
	Rectangle _myTextureRect; // The rect maps to the whole texture.

	bool _isLoaded;
};

}