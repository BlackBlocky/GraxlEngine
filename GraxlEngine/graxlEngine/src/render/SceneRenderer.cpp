#include <graxlEngine/render/SceneRenderer.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <optional>

namespace graxl {
	SceneRenderer::SceneRenderer(Vector2 __aspectRatio, int __baseWidthPixels, int __basePixelsPerUnit)
		: _aspectRatioX((int)__aspectRatio.x), _aspectRatioY((int)__aspectRatio.y), _basePixelsPerUnit(__basePixelsPerUnit),
		_baseWindowWidth(__baseWidthPixels),
		// Casting first to double for max presicion
		_baseWindowHeight(static_cast<int>(static_cast<double>(__baseWidthPixels) / static_cast<double>(__aspectRatio.x) * static_cast<double>(__aspectRatio.y)))
	{
		camera2d.offset = { 0.0f, 0.0f };
		camera2d.rotation = 0.0f;
		camera2d.zoom = 1.0f;

		camera2d.target = { 0.0f, 0.0f };
	}

	void SceneRenderer::renderSceneToRendererBuffer(const Scene& scene)
	{
		// Calculate all the scaling etc for the aspect ratio if needed
		Vector2Int readedSize = { GetScreenWidth(), GetScreenHeight() };
		if (readedSize.x != _currentScreenSize.x || readedSize.y != _currentScreenSize.y) _recalculateAspectRatio(readedSize);

		// Draw the Background and GameObjects to A
		BeginTextureMode(_renderTexA); // --- Rendering Base Scene ---

		ClearBackground(DARKGRAY);
		//DrawRectangle(_blackBorderSize.x / 2, _blackBorderSize.y / 2, _viewportAspectRatioSize.x, _viewportAspectRatioSize.y, GRAY); // Shows the viewport

		// Set the camera pos to the center of the screen, and move the camera back, so it everything is centered again
		camera2d.target = { (float)_viewportAspectRatioSize.x / 2.0f, (float)_viewportAspectRatioSize.y / 2.0f };
		camera2d.offset = { (float)_viewportAspectRatioSize.x / 2.0f, (float)_viewportAspectRatioSize.y / 2.0f };

		// -- Render the GameObjects --

		// Put every item that need to be rendered into a queue
		_renderQueueMap_Base.clear();

		for (const auto& gameObject : scene.getAllGameObjectsConst()) {
			const GameObject* const goPtr = gameObject.lock().get();
			_renderQueueMap_Base[goPtr->drawOrder].push_back([this, goPtr]() {
				this->_drawGameObject(*goPtr);
			});
		}

		// Process the render queue
		BeginMode2D(camera2d);
		for (const auto& drawlayerMapPair : _renderQueueMap_Base) {
			for (const auto& drawFunction : drawlayerMapPair.second) {
				drawFunction();
			}
		}
		EndMode2D();
		EndTextureMode(); // --- Base Scene was drawn ---

		// -- Done rendering Gameobjects --


		// --- Shader Rendering ---
		// A queue is not needed, because shaders are presorted by priority/orderInLayer. (Done in Scene.cpp)

		// Update all GLSL shaders uniform data
		for (const std::weak_ptr<GameShader>& gameShader : scene.getShaderListSorted()) {
			auto gameShaderSharedPtr = gameShader.lock();

			if (gameShaderSharedPtr->getType() == GameShaderType::GLSL) {
				_updateShaderUniformData(gameShaderSharedPtr.get(), &gameShaderSharedPtr->getShaderUniformData());
			}
		}

		// Apply Shaders/Effects
		// For allowing multiple shaders, RenderTexture A and B are gonna swap until and draw on each other, until every shader is drawn
		// NOTE: The shader array is alrady sorted by priority/orderInLayer in the Scene.cpp. So we can just iterate through it, without the need to sort it
		{
			// We start at A because everything else is also drawn on A
			RenderTexture2D* textureSwapA = &_renderTexA; // In A will always be the last drawn result
			RenderTexture2D* textureSwapB = &_renderTexB;

			for (const std::weak_ptr<GameShader>& gameShader : scene.getShaderListSorted()) {
				auto gameShaderSharedPtr = gameShader.lock();
				
				if (gameShaderSharedPtr->getType() != GameShaderType::GLSL) continue; // Only GLSL are drawn here!
				BeginTextureMode(*textureSwapB);
				if (const std::optional<Shader>& shaderData = gameShaderSharedPtr->getShader()) {
					BeginShaderMode(shaderData.value()); // Get the RaylibShader out of the GameShader
					_drawAToB(*textureSwapA, *textureSwapB, true);
					EndShaderMode();
				}
				else {
					spdlog::warn("SceneRenderer: ShaderData of GLSL GameShader \"{}\" is nullopt! Shader wont be drawn.", gameShaderSharedPtr->getName());
				}
				EndTextureMode();

				// Swap Swappers
				std::swap(textureSwapA, textureSwapB); // Basically moving the render result to A
			}

			// Finally drawing the result to the _rendererResult texture
			BeginTextureMode(_rendererResult);
			_drawAToB(*textureSwapA, _rendererResult, false);
			EndTextureMode();
		}
		// --- Shader Rendering done ---	


		// Draw UI on top of the _rendererResult

		
		// Draw the black borders (if there are any)
		if (_blackBorderSize.x > 0) {
			DrawRectangle(0, 0, _blackBorderSize.x / 2, _currentScreenSize.y, BLACK);
			DrawRectangle(_currentScreenSize.x - (_blackBorderSize.x / 2), 0, _blackBorderSize.x / 2, _currentScreenSize.y, BLACK);
		}
		if (_blackBorderSize.y > 0) {
			DrawRectangle(0, 0, _currentScreenSize.x, _blackBorderSize.y / 2, BLACK);
			DrawRectangle(0, _currentScreenSize.y - (_blackBorderSize.y / 2), _currentScreenSize.x, _blackBorderSize.y / 2, BLACK);
		}

	}

	void SceneRenderer::renderBufferToScreen()
	{
		// Because we are in drawing mode in this function, we can just draw the result buffer to the screen
		// (Using the _blackBorderSize offset, because the screen is maybe not the target aspect ratio, what ever other target resoulution)
		DrawTexture(_rendererResult.texture, _blackBorderSize.x / 2, _blackBorderSize.y / 2, WHITE);
	}

	void SceneRenderer::renderBlank()
	{
		ClearBackground(BLACK);
	}



	void SceneRenderer::unloadRenderTextures()
	{
		spdlog::info("SceneRenderer: Finally unloading RenderTextures used for rendering...");
		if (IsRenderTextureValid(_rendererResult)) {
			UnloadRenderTexture(_rendererResult);
			UnloadRenderTexture(_renderTexA);
			UnloadRenderTexture(_renderTexB);
		}
	}


	void SceneRenderer::_recalculateAspectRatio(Vector2Int& newScreenSize)
	{
		_currentScreenSize = newScreenSize;

		// Calculate the black box
		_blackBorderSize = _calcBlackBorderSize(newScreenSize);
		// Calculate the size of the actual rendered gameplay area, aka. viewport
		_viewportAspectRatioSize = { newScreenSize.x - _blackBorderSize.x, newScreenSize.y - _blackBorderSize.y }; // The size of the actual rendered gameplay
		// Calc the scale factor for the game. If its larger than the base window size, or smaller. The scaling applies only the the viewport, not the whole window.
		_viewportScale = { (float)_viewportAspectRatioSize.x / (float)_baseWindowWidth, (float)_viewportAspectRatioSize.y / (float)_baseWindowHeight };

		// Calc the new pixels per unit value
		_scaledPixelsPerUnit = { _viewportScale.x * (float)_basePixelsPerUnit, _viewportScale.y * (float)_basePixelsPerUnit };


		spdlog::info("SceneRenderer: Begin recreating renderTextures for rendering...");
		// Unload all old renderTextures
		if (IsRenderTextureValid(_rendererResult)) {
			UnloadRenderTexture(_rendererResult);
			UnloadRenderTexture(_renderTexA);
			UnloadRenderTexture(_renderTexB);
		}
		// Create/Load new rendertextures with the according viewport size
		_rendererResult = LoadRenderTexture(_viewportAspectRatioSize.x, _viewportAspectRatioSize.y);
		_renderTexA = LoadRenderTexture(_viewportAspectRatioSize.x, _viewportAspectRatioSize.y);
		_renderTexB = LoadRenderTexture(_viewportAspectRatioSize.x, _viewportAspectRatioSize.y);
		spdlog::info("SceneRenderer: Recreating renderTextures done.");
		// Setting the filters to POINT (So basically no filter)
		SetTextureFilter(_rendererResult.texture, TEXTURE_FILTER_POINT);
		SetTextureFilter(_renderTexA.texture, TEXTURE_FILTER_POINT);
		SetTextureFilter(_renderTexB.texture, TEXTURE_FILTER_POINT);


		spdlog::debug("SceneRenderer: Recalculated Aspect Ratio: Viewport({}, {}), Scale({}, {}), PixelsPerUnit({}, {})",
			_viewportAspectRatioSize.x, _viewportAspectRatioSize.y, _viewportScale.x, _viewportScale.y, _scaledPixelsPerUnit.x, _scaledPixelsPerUnit.y);
	}

	// Returns the total size of the border. Example left + right border size together.
	SceneRenderer::Vector2Int SceneRenderer::_calcBlackBorderSize(Vector2Int& windowSize) {
		int subAspectRatioWidth = windowSize.x / _aspectRatioX;
		int subAspectRatioHeight = windowSize.y / _aspectRatioY;

		if (subAspectRatioWidth * _aspectRatioY <= windowSize.y) {
			int blackBorderHeight = windowSize.y - (subAspectRatioWidth * _aspectRatioY);
			return { 0, blackBorderHeight };
		}
		else {
			int blackBorderWidth = windowSize.x - (subAspectRatioHeight * _aspectRatioX);
			return { blackBorderWidth, 0 };
		}
	}

	SceneRenderer::Vector2Int SceneRenderer::_calcViewportPixelPos(const Vector2& pos, const Vector2Int& sizePixels)
	{
		return {
			(int)(pos.x * _scaledPixelsPerUnit.x),
			-((int)(pos.y * _scaledPixelsPerUnit.y)) + _viewportAspectRatioSize.y - sizePixels.y
		};
	}

	SceneRenderer::Vector2Int SceneRenderer::_calcViewportPixelSize(const Vector2& size)
	{
		return {
			(int)(size.x * _scaledPixelsPerUnit.x),
			(int)(size.y * _scaledPixelsPerUnit.y)
		};
	}

	// Only used for GLSL shaders. Others shaders will just reading the intensity in render-prozess
	// NOTE: The shader ONLY tranfers data from the shaderUniformData to OpenGL,
	//		 it doesn't generate data, like globalTime. The only exception is the resolution, because its only defined by the renderer
	void SceneRenderer::_updateShaderUniformData(const GameShader* const targetShader, const GameShader::ShaderUniformData* const uniformData)
	{
		if(!targetShader->getShader().has_value()) {
			spdlog::warn("SceneRenderer: Tried to update ShaderUniformData of Shader \"{}\", but the shader isnt loaded or a GLSL shader!", targetShader->getName());
			return;
		}

		const Shader& shaderRef = targetShader->getShader().value();

		// Update the default values
		int shaderLocation_GlobalTime = GetShaderLocation(shaderRef, "globalTime");
		SetShaderValue(shaderRef, shaderLocation_GlobalTime, (const void*)&uniformData->globalTime, ShaderUniformDataType::SHADER_UNIFORM_FLOAT);
		int shaderLocation_SongTime = GetShaderLocation(shaderRef, "songTime");
		SetShaderValue(shaderRef, shaderLocation_SongTime, (const void*)&uniformData->songTime, ShaderUniformDataType::SHADER_UNIFORM_FLOAT);
		int shaderLocation_Intensity = GetShaderLocation(shaderRef, "intensity");
		SetShaderValue(shaderRef, shaderLocation_Intensity, (const void*)&uniformData->intensity, ShaderUniformDataType::SHADER_UNIFORM_FLOAT);

		// Update static values set by the renderer
		int shaderLocation_Resolution = GetShaderLocation(shaderRef, "resolution");
		Vector2 resFloat = { (float)_viewportAspectRatioSize.x, (float)_viewportAspectRatioSize.y };
		SetShaderValue(shaderRef, shaderLocation_Resolution, (const void*)&resFloat, ShaderUniformDataType::SHADER_UNIFORM_VEC2);

		// Update the custom values, based on the amount of custom values the shader has
		for (int i = 0; i < targetShader->getCustomValuesAmount(); i++) {
			std::string uniformName = "cValue" + std::to_string(i);
			int shaderLocation_CustomValue = GetShaderLocation(shaderRef, uniformName.c_str());
			SetShaderValue(shaderRef, shaderLocation_CustomValue, (const void*)&uniformData->customValues[i], ShaderUniformDataType::SHADER_UNIFORM_FLOAT);
		}
	}



	// +++++++++++++++++
	// DRAWING FUNCTIONS
	// +++++++++++++++++

	void SceneRenderer::_drawAToB(RenderTexture2D& source, RenderTexture2D& target, bool flipY)
	{
		// Note: We dont need to use the blackBorder offset, because all RenderTexture2D should be just the viewport size
		// Note: the target is actually not used... but i leave to make thing more clear i guess ikd xD
		
		if (flipY) {
			// Flipping the y axis because on render Textures (0,0) is on the bottom left, while on the screen its on the top left on (0,0)
			// This is only a issue in ShaderMode, because the shader basically flip the result; So we flip
			DrawTextureRec(source.texture,
				Rectangle{ 0, 0, (float)source.texture.width, -(float)source.texture.height },
				Vector2{ 0, 0 },
				WHITE
			);
		}
		else {
			// No corrections done. We dont need a correction if we dont do draw via ShaderMode
			DrawTexture(source.texture, 0, 0, WHITE);
		}			
	}

	void SceneRenderer::_drawSquare(const Vector2& pos, const Vector2& size, const Color& color)
	{
		Vector2Int windowPixelSize = _calcViewportPixelSize(size);
		Vector2Int windowPixelPos = _calcViewportPixelPos(pos, windowPixelSize);
		

		DrawRectangle(windowPixelPos.x, windowPixelPos.y, windowPixelSize.x, windowPixelSize.y, color);
	}

	void SceneRenderer::_drawGameObject(const GameObject& gameObject)
	{
		// If the gameObjects texture is a nullptr/invalid, just draw a magenta box
		if (const auto& tex = gameObject.texture.lock()) {
			Vector2Int goSizePixels = _calcViewportPixelSize(gameObject.size);
			Vector2Int drawPosPixels = _calcViewportPixelPos(gameObject.position, goSizePixels);

			const Texture2D& drawTexture = tex->getTexture();
			const Rectangle& drawTextureRect = tex->getTextureRect();

			DrawTexturePro(drawTexture, drawTextureRect,
				{ (float)drawPosPixels.x, (float)drawPosPixels.y, (float)goSizePixels.x, (float)goSizePixels.y }, { 0.0f, 0.0f },
				0.0f, WHITE);
		}
		else {
			// texture is nullptr
			_drawSquare(gameObject.size, gameObject.position, MAGENTA);
		}
		
	}

	

}