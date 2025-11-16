#pragma once

#include <vector>
#include <memory>

#include <graxlEngine/render/GameShader.h>
#include <graxlEngine/render/GameTexture.h>

namespace graxl {

	class ImGUIObjectRenderer {
	public:
		// Renders all connected imgui objects
		// Make sure youre in the ImGui render mode before calling this function!
		void tick();

		// Shader
		void addShaderToRenderList(const std::weak_ptr<GameShader>& shader_wp);
		void removeShaderToRenderList(const std::weak_ptr<GameShader>& shader_wp);

		// Texture
		void addTextureToRenderList(const std::weak_ptr<GameTexture>& texture_wp);
		void removeTextureToRenderList(const std::weak_ptr<GameTexture>& texture_wp);

	private:
		std::vector<std::weak_ptr<GameShader>> _connectedShaderDatas;
		std::vector<std::weak_ptr<GameTexture>> _connectedTextureDatas;
		
		void _renderShaderDataWindow(GameShader* const shader);
		void _renderTextureDataWindow(GameTexture* const texture);

		// Checks if the check item is in the containsList.
		// Returns the iterator to the item if found, else returns containsList.end()
		template<typename T>
		typename std::vector<std::weak_ptr<T>>::const_iterator _checkIfInList(const std::weak_ptr<T>& check, const std::vector<std::weak_ptr<T>>& containsList) noexcept;

		// Removes every expired weak_ptr from the given list
		template<typename T>
		void _removeExpiredFromList(std::vector<std::weak_ptr<T>>& removeList);
	};

}