#include "graxlEngine/ui/ImGUIObjectRenderer.h"

#include <imgui.h>

#include <typeinfo>

namespace graxl {

	void ImGUIObjectRenderer::tick()
	{
		// Remove all dead pointer first (If there are any)
		_removeExpiredFromList<GameShader>(_connectedShaderDatas);
		_removeExpiredFromList<GameTexture>(_connectedTextureDatas);

		// Render all shader datas. And making double sure we have no dead pointer
		for (const auto& shaderWp : _connectedShaderDatas) {
			if(auto shaderSp = shaderWp.lock()) {
				_renderShaderDataWindow(shaderSp.get());
			}
		}
		for(const auto& textureWp : _connectedTextureDatas) {
			if(auto textureSp = textureWp.lock()) {
				_renderTextureDataWindow(textureSp.get());
			}
		}
	}

	void ImGUIObjectRenderer::addShaderToRenderList(const std::weak_ptr<GameShader>& shader_wp)
	{
		// Check if the weak_ptr is valid
		if(shader_wp.expired()) {
			spdlog::warn("ImGUIObjectRenderer: Tried to add shader to ImGUI render list, but the weak_ptr is expired!");
			return;
		}

		// Check if the shader is already in the list
		auto foundIt = _checkIfInList(shader_wp, _connectedShaderDatas);

		if (foundIt != _connectedShaderDatas.end()) {
			spdlog::warn("ImGUIObjectRenderer: Tried to add shader \"{}\" to ImGUI render list, but it is already in the list!", shader_wp.lock()->getName());
			return;
		}
		else {
			// Add the shader to the list as an weak pointer
			_connectedShaderDatas.push_back(shader_wp);
			spdlog::trace("ImGUIObjectRenderer: Added shader \"{}\" to ImGUI render list.", shader_wp.lock()->getName());
		}
	}

	void ImGUIObjectRenderer::removeShaderToRenderList(const std::weak_ptr<GameShader>& shader_wp)
	{
		// Check if the weak_ptr is valid
		if(shader_wp.expired()) {
			spdlog::warn("ImGUIObjectRenderer: Tried to remove shader from ImGUI render list, but the weak_ptr is expired!");
			return;
		}

		// Find the iterator pos of the shader_sp arguemnt
		auto foundIt = _checkIfInList(shader_wp, _connectedShaderDatas);

		// Remove if found, else log warning
		if (foundIt != _connectedShaderDatas.end()) {
			_connectedShaderDatas.erase(foundIt);
			spdlog::trace("ImGUIObjectRenderer: Removed shader \"{}\" from ImGUI render list.", shader_wp.lock()->getName());
		}
		else {
			spdlog::warn("ImGUIObjectRenderer: Tried to remove shader \"{}\" from ImGUI render list, but it was not found in the list!", shader_wp.lock()->getName());
		}
	}

	void ImGUIObjectRenderer::addTextureToRenderList(const std::weak_ptr<GameTexture>& texture_wp)
	{
		// Check if the weak_ptr is valid
		if (texture_wp.expired()) {
			spdlog::warn("ImGUIObjectRenderer: Tried to add texture to ImGUI render list, but the weak_ptr is expired!");
			return;
		}

		// Check if the texture is already in the list
		auto foundIt = _checkIfInList<GameTexture>(texture_wp, _connectedTextureDatas);

		// Add if found, else log warning
		if (foundIt != _connectedTextureDatas.end()) {
			spdlog::warn("ImGUIObjectRenderer: Tried to add texture \"{}\" to ImGUI render list, but it is already in the list!", texture_wp.lock()->name);
			return;
		}
		else {
			_connectedTextureDatas.push_back(texture_wp);
			spdlog::trace("ImGUIObjectRenderer: Added texture \"{}\" to ImGUI render list.", texture_wp.lock()->name);
		}
	}

	void ImGUIObjectRenderer::removeTextureToRenderList(const std::weak_ptr<GameTexture>& texture_wp)
	{
		// Check if the weak_ptr is valid
		if (texture_wp.expired()) {
			spdlog::warn("ImGUIObjectRenderer: Tried to remove texture to ImGUI render list, but the weak_ptr is expired!");
			return;
		}

		// Check if the texture is in the list
		auto foundIt = _checkIfInList<GameTexture>(texture_wp, _connectedTextureDatas);

		// Remove if found, else log warning
		if (foundIt != _connectedTextureDatas.end()) {
			_connectedTextureDatas.erase(foundIt);
			spdlog::trace("ImGUIObjectRenderer: Removed texture \"{}\" from ImGUI render list.", texture_wp.lock()->name);
		}
		else {
			spdlog::warn("ImGUIObjectRenderer: Tried to remove texture \"{}\" from ImGUI render list, but it was not found in the list!", texture_wp.lock()->name);
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

	void ImGUIObjectRenderer::_renderTextureDataWindow(GameTexture* const texture)
	{
		ImGui::Begin(texture->name.c_str());
		ImGui::Text("Texture Name: %s", texture->name.c_str());
		ImGui::Text("Texture Size: %dx%d", texture->getTexture().width, texture->getTexture().height);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImVec2 imgSize = ImVec2((float)texture->getTexture().width, (float)texture->getTexture().height);
		ImGui::Image((void*)(intptr_t)texture->getTexture().id, imgSize);
		ImGui::End();
	}

	template std::vector<std::weak_ptr<GameShader>>::const_iterator ImGUIObjectRenderer::_checkIfInList<GameShader>(const std::weak_ptr<GameShader>&, const std::vector<std::weak_ptr<GameShader>>&) noexcept;
	template std::vector<std::weak_ptr<GameTexture>>::const_iterator ImGUIObjectRenderer::_checkIfInList<GameTexture>(const std::weak_ptr<GameTexture>&, const std::vector<std::weak_ptr<GameTexture>>&) noexcept;

	template<typename T>
	typename std::vector<std::weak_ptr<T>>::const_iterator ImGUIObjectRenderer::_checkIfInList(const std::weak_ptr<T>& check, const std::vector<std::weak_ptr<T>>& containsList) noexcept {
		typename std::vector<std::weak_ptr<T>>::const_iterator foundIt = std::find_if(containsList.begin(), containsList.end(), [&check](const auto& currentIt) {
			if(auto cList = currentIt.lock()) {
				if(auto cCheck = check.lock()) {
					return cList == cCheck;
				}
			}
			return false;
		});

		return foundIt;
	}

	template void ImGUIObjectRenderer::_removeExpiredFromList(std::vector<std::weak_ptr<GameShader>>& removeList);
	template void ImGUIObjectRenderer::_removeExpiredFromList(std::vector<std::weak_ptr<GameTexture>>& removeList);

	template<typename T>
	void ImGUIObjectRenderer::_removeExpiredFromList(std::vector<std::weak_ptr<T>>& removeList)
	{
		removeList.erase(
			std::remove_if(removeList.begin(), removeList.end(), [](const auto& weakItem) {
				if (weakItem.expired()) {
					spdlog::warn("ImGUIObjectRenderer: Removed expired item from ImGUI render list. Type: {}", typeid(T).name());
					return true;
				}
				else return false;
			}),
			removeList.end()
		);
	}
}