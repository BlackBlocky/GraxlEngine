#pragma once

#include <map>
#include <functional>
#include <vector>

#include "graxlEngine/scene/Scene.h"
#include <graxlEngine/scene/GameObject.h>
#include <graxlEngine/GVec.h>

#include "raylib.h"

namespace graxl {

class SceneRenderer {
public:
	SceneRenderer(GVec2i __aspectRatio, int __baseWidthPixels, int __basePixelsPerUnit);

	// In order to call the function, raylib must be initialized AND raylib CANNOT be in render mode.
	void renderSceneToRendererBuffer(const Scene& scene);
	// In order to call the function, raylib must be initialized AND raylib MUST be in render mode.
	void renderBufferToScreen();
	// In order to call the function, raylib must be initialized AND raylib MUST be in render mode. It will also just draw a gray background
	void renderBlank();

	// This should be called when the renderer isnt needed anymore
	void unloadRenderTextures();

	// Getter
	const int getBaseWindowWidth() const { return _baseWindowWidth; };
	const int getBaseWindowHeight() const { return _baseWindowHeight; };

	Camera2D camera2d;

private:
	void _recalculateAspectRatio(GVec2i& newWindowSize);
	GVec2i _calcBlackBorderSize(GVec2i& currentWindowSize);

	GVec2i _calcViewportPixelPos(const GVec2f& pos, const GVec2i& sizePixels);
	GVec2i _calcViewportPixelSize(const GVec2f& size);
	
	void _updateShaderUniformData(const GameShader* const targetShader, const GameShader::ShaderUniformData* const uniformData);

	// IMPORTANT: RenderTextureMode must be set to the TARGET-RenderTexture2D before calling that function! (Becase optimization)
	void _drawAToB(RenderTexture2D& source, RenderTexture2D& target, bool flipY);

	void _drawSquare(const GVec2f& pos, const GVec2f& size, const Color& color);
	void _drawGameObject(const GameObject& gameObject);

	const int _baseWindowWidth = 1920; // (Default value, not really used)
	const int _baseWindowHeight = 1080; // (Default value, not really used)

	const int _aspectRatioX = 16; // (Default value, not really used)
	const int _aspectRatioY = 9; // (Default value, not really used)

	GVec2i _currentScreenSize{ 0, 0 };
	GVec2i _blackBorderSize{ 0, 0 };
	GVec2i _viewportAspectRatioSize{ 0, 0 };
	GVec2f _viewportScale{ 0.0f, 0.0f };
	
	const int _basePixelsPerUnit = 100; // (Default value, not really used)
	GVec2f _scaledPixelsPerUnit{ 0, 0 }; // Using an float because we need precision on that value. Because even small roundings will stack up

	RenderTexture2D _rendererResult; // This rendertexture is used, to store the renderresult in it, and then later drawing it on the screen
	RenderTexture2D _renderTexA; // A and B are used to render stuff on each other, until e.g. all shaders are drawn
	RenderTexture2D _renderTexB;

	std::map<const int, std::vector<std::function<void()>>> _renderQueueMap_Base;
};

}