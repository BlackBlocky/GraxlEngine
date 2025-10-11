#include "graxlEngine/ui/ImGUIObjectRenderer.h"

#include <imgui.h>

namespace graxl {

	void ImGUIObjectRenderer::tick()
	{
		// Remove all dead pointer first (If there are any)
		_connectedShaderDatas.erase(
			std::remove_if(_connectedShaderDatas.begin(), _connectedShaderDatas.end(),
				[](const auto& weakShader) {
					if (weakShader.expired()) {
						spdlog::warn("ImGUIObjectRenderer: Shader {} was destroyed, but is still in the ImGUI Render Loop! Removing shader from list...");
						return true;
					}
					else return false;
				}
			),
			_connectedShaderDatas.end()
		);

		// Render all shader datas. And making double sure we have no dead pointer
		for (const auto& shaderWp : _connectedShaderDatas) {
			if(auto shaderSp = shaderWp.lock()) {
				_renderShaderDataWindow(shaderSp.get());
			}
		}
	}

	void ImGUIObjectRenderer::addShaderToRenderList(const std::weak_ptr<GameShader>& shader_wp)
	{
		// Check if the shader is already in the list
		for (const auto& weakShader : _connectedShaderDatas) {
			if (auto sharedShader = weakShader.lock()) {
				if (sharedShader == shader_wp.lock()) {
					spdlog::warn("ImGUIObjectRenderer: Tried to add shader \"{}\" to ImGUI render list, but it is already in the list!", shader_wp.lock()->getName());
					return;
				}
			}
		}

		// Add the shader to the list as an weak pointer
		_connectedShaderDatas.push_back(shader_wp);
		spdlog::trace("ImGUIObjectRenderer: Added shader \"{}\" to ImGUI render list.", shader_wp.lock()->getName());
	}

	void ImGUIObjectRenderer::removeShaderToRenderList(const std::weak_ptr<GameShader>& shader_wp)
	{
		// Find the iterator pos of the shader_sp arguemnt
		auto shaderSpFoundIt = std::find_if(_connectedShaderDatas.begin(), _connectedShaderDatas.end(),
			[&shader_wp](const auto& weakShader) {
				if (auto sharedShader = weakShader.lock()) {
					return sharedShader == shader_wp.lock();
				}
				return false;
			});

		// Remove if found, else log warning
		if (shaderSpFoundIt != _connectedShaderDatas.end()) {
			_connectedShaderDatas.erase(shaderSpFoundIt);
			spdlog::trace("ImGUIObjectRenderer: Removed shader \"{}\" from ImGUI render list.", shader_wp.lock()->getName());
		}
		else {
			spdlog::warn("ImGUIObjectRenderer: Tried to remove shader \"{}\" from ImGUI render list, but it was not found in the list!", shader_wp.lock()->getName());
		}
	}

	void ImGUIObjectRenderer::_renderShaderDataWindow(GameShader* const shader)
	{
		GameShader::ShaderUniformData* const shaderUniformData = shader->getShaderUniformDataPtr().get();

		ImGui::Begin(shader->getName().c_str());
		ImGui::Text("Global Time %.2f", shaderUniformData->globalTime);
		ImGui::SliderFloat("Song Time", &shaderUniformData->songTime, 0.0f, 100.0f);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::SliderFloat("Intensity", &shaderUniformData->intensity, 0.0f, 1.0f);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		for (int i = 0; i < shaderUniformData->customValues.size(); i++) {
			std::string label = std::format("cValue{}", i);
			ImGui::SliderFloat(label.c_str(), &shaderUniformData->customValues[i], 0.0f, 100.0f);
		}

		ImGui::End();
	}

}