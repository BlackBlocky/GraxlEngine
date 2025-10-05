#pragma once

#include <vector>
#include <memory>

#include <graxlEngine/render/GameShader.h>

namespace graxl {

	class ImGUIObjectRenderer {
	public:
		// Renders all connected imgui objects
		// Make sure youre in the ImGui render mode before calling this function!
		void tick();

		void addShaderToRenderList(const std::shared_ptr<GameShader>& shader_sp);
		void removeShaderToRenderList(const std::shared_ptr<GameShader>& shader_sp);

	private:
		std::vector<std::weak_ptr<GameShader>> _connectedShaderDatas;

		void _renderShaderDataWindow(GameShader* const shader);
	};

}