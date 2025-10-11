#pragma once

#include <map>
#include <functional>
#include <vector>

#include "graxlEngine/scene/Scene.h"
#include <graxlEngine/scene/GameObject.h>

#include "raylib.h"

namespace graxl {

class SceneRenderer {
public:
	SceneRenderer();

	// In order to call the function, raylib must be initialized AND raylib CANNOT be in render mode.
	void renderSceneToRendererBuffer(const Scene& scene);
	// In order to call the function, raylib must be initialized AND raylib MUST be in render mode.
	void renderBufferToScreen();
	// In order to call the function, raylib must be initialized AND raylib MUST be in render mode. It will also just draw a gray background
	void renderBlank();

	// This should be called when the renderer isnt needed anymore
	void unloadRenderTextures();

	Camera2D camera2d;

private:
	struct Vector2Int {
		int x;
		int y;
	};;

	void _recalculateAspectRatio(Vector2Int& newWindowSize);
	Vector2Int _calcBlackBorderSize(Vector2Int& currentWindowSize);

	Vector2Int _calcViewportPixelPos(const Vector2& pos, const Vector2Int& sizePixels);
	Vector2Int _calcViewportPixelSize(const Vector2& size);
	
	void _updateShaderUniformData(const GameShader* const targetShader, const GameShader::ShaderUniformData* const uniformData);

	// IMPORTANT: RenderTextureMode must be set to the TARGET-RenderTexture2D before calling that function! (Becase optimization)
	void _drawAToB(RenderTexture2D& source, RenderTexture2D& target, bool flipY);

	void _drawSquare(const Vector2& pos, const Vector2& size, const Color& color);
	void _drawGameObject(const GameObject& gameObject);

	const int _baseWindowWidth = 1920;
	const int _baseWindowHeight = 1080;

	const int _aspectRatioX = 16;
	const int _aspectRatioY = 9;

	Vector2Int _currentScreenSize{ 0, 0 };
	Vector2Int _blackBorderSize{ 0, 0 };
	Vector2Int _viewport169Size{ 0, 0 };
	Vector2 _viewportScale{ 0.0f, 0.0f };
	
	const int _basePixelsPerUnit = 100;
	Vector2 _scaledPixelsPerUnit{ 0, 0 }; // Using an float because we need precision on that value. Because even small roundings will stack up

	RenderTexture2D _rendererResult; // This rendertexture is used, to store the renderresult in it, and then later drawing it on the screen
	RenderTexture2D _renderTexA; // A and B are used to render stuff on each other, until e.g. all shaders are drawn
	RenderTexture2D _renderTexB;

	std::map<const int, std::vector<std::function<void()>>> _renderQueueMap_Base;
};

}