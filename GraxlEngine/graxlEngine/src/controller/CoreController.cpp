#include "graxlEngine/controller/CoreController.h"

#include <format>

#include <raylib.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <rlImGui.h>
#include <magic_enum/magic_enum.hpp>

#include <graxlEngine/render/GameShader.h>
#include <graxlEngine/event/GuiSwitchSceneElementDebugWindow.h>
#include <graxlEngine/event/GameStateSwitchEvent.h>
#include <graxlEngine/GraxlConstants.h>

namespace graxl {

	int CoreController::init(const std::string& windowTitle, GVec2i aspectRatio, int baseWindowWidthPixels, int pixelsPerUnit, int framerate, bool debugMode)
	{
		_debugMode = debugMode;

		// Setup the renderes aspect ratio, base resoultion. (Done first because a lot depends on the base resoultion)
		_sceneRenderer.emplace(aspectRatio, baseWindowWidthPixels, pixelsPerUnit);

		spdlog::info("Init Graxl with title:{}, AspectRatio:{}/{}, BaseRes:{}/{}, PixelsPerUnity:{}, Framerate:{}, DebugMode:{}", 
			windowTitle, aspectRatio.x(), aspectRatio.y(), _sceneRenderer->getBaseWindowWidth(), _sceneRenderer->getBaseWindowHeight(), pixelsPerUnit, framerate, debugMode);

		// Init every system for the Engine
		if (this->_initWindow(framerate, windowTitle, {(float)_sceneRenderer->getBaseWindowWidth(), (float)_sceneRenderer->getBaseWindowHeight()}) != 0) return 1;
		if (this->_initInputSystem() != 0) return 1;
		if (this->_startImGUI() != 0) return 1;
		if (this->_initConsole() != 0) return 1;

		return 0;
	}

	int CoreController::_initWindow(int framerate, const std::string& title, GVec2i res)
	{
		// Move Raylib logging to spdlog logging
		SetTraceLogCallback(_raylibLogCallback);

		spdlog::info("CoreController: Initializing window...");

		SetConfigFlags(FLAG_WINDOW_RESIZABLE);

		InitWindow(res.x(), res.y(), title.c_str());
		SetTargetFPS(framerate);

		_windowInitialized = true;
		spdlog::info("CoreController: Window initialized successfully.");
		return 0;
	}

	// Only used by Raylib to log messages, we use spdlog to handle the logging
	// The Log level INFO will be turned to DEBUG when entering the initGameLoog() funtion (Because ínfo log spam)
	bool __raylibInfoLogToDebug = false;
	void CoreController::_raylibLogCallback(int msgType, const char* text, va_list args) {
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), text, args);

		if (msgType == LOG_INFO && __raylibInfoLogToDebug) {
			msgType = LOG_DEBUG;
		}

		switch (msgType) {
		case LOG_INFO:
			spdlog::info("[RAYLIB] {}", buffer);
			break;
		case LOG_WARNING:
			spdlog::warn("[RAYLIB] {}", buffer);
			break;
		case LOG_ERROR:
			spdlog::error("[RAYLIB] {}", buffer);
			break;
		case LOG_DEBUG:
		default:
			spdlog::debug("[RAYLIB] {}", buffer);
			break;
		}
	}

	int CoreController::_initInputSystem()
	{
		spdlog::info("CoreController: Initializing input system...");

		_inputSystem = InputSystem();

		_inputSystemInitialized = true;
		spdlog::info("CoreController: Input system initialized successfully.");
		return 0;
	}

	int CoreController::_startImGUI()
	{
		if (!_debugMode) {
			spdlog::info("CoreController: Dont start ImGUI, because Debug-Mode is disabled");
			return 0;
		}
		if (!_windowInitialized) {
			spdlog::critical("CoreController: Cannot start ImGUI, window not initialized.");
			return 1;
		}
		spdlog::info("CoreController: Starting ImGUI...");

		rlImGuiSetup(true); // sets up ImGui with ether a dark or light default theme
		_imguiObjectRenderer = ImGUIObjectRenderer();

		_imguiInitialized = true;
		spdlog::info("CoreController: ImGUI started successfully.");
		return 0;
	}

	int CoreController::_initConsole()
	{
		if (!_debugMode) {
			spdlog::info("CoreController: Dont initialize console, because Debug-Mode is disabled");
			return 0;
		}
		if (!_imguiInitialized) {
			spdlog::critical("CoreController: Cannot initialize console, ImGUI not initialized.");
			return 1;
		}
		spdlog::info("CoreController: Initializing console...");

		_consoleController.emplace(*this);

		_consoleInitialized = true;
		spdlog::info("CoreController: Console initialized successfully.");
		return 0;
	}

	void graxl::CoreController::addGameState(GameState&& newGameState)
	{
		if (GAME_STATES.find(newGameState.id) != GAME_STATES.end()) {
			spdlog::warn("CoreController: Tried to add GameState \"{}\", but it already exists! Overwriting...", newGameState.id);
		}

		std::string newGameStateIdCopy = newGameState.id; // Making a copy of the GameStateId because its going to be moved, then newGameState.id will be empty
		GAME_STATES[newGameStateIdCopy] = std::make_shared<const GameState>(std::move(newGameState));
		spdlog::debug("CoreController: Added GameState \"{}\"", newGameStateIdCopy);
	}

	std::weak_ptr<Scene> CoreController::createEmptyScene(const std::string sceneName)
	{
		spdlog::info("CoreController: Creating empty scene \"{}\"...", sceneName);
		_switchToScene(Scene(sceneName, "graxl.created.emptyScene"));

		return _currentScene;
	}

	void CoreController::createSceneDefinition(const std::string& id, const std::string& name, std::function<void(Scene& scene)> buildFunction)
	{
		if (_sceneDefs.contains(id)) {
			spdlog::error("CoreController: Tried to add Scene-Definition {} with id {}, but there is already a Scene-Definition with that id!", name, id);
			return;
		}

		spdlog::debug("CoreController: Adding Scene-Definition {} with id {}", name, id);
		_sceneDefs.emplace(id, SceneDef(id, name, buildFunction));
	}

	std::weak_ptr<Scene> CoreController::buildAndSwitchToScene(const std::string& id)
	{
		if (!_sceneDefs.contains(id)) {
			spdlog::error("CoreController: Tried to buildAndSwitch to Scene-Id {}, but this id isn't registered!", id);
			return std::weak_ptr<Scene>();
		}

		_switchToScene(_sceneDefs[id].build());

		return _currentScene;
	}


	int CoreController::initGameLoop(std::string startingState)
	{
		// Make sure everything is settep up correctly before starting the game loop
		if (!_windowInitialized) {
			spdlog::critical("CoreController: Cannot initialize game loop, window not initialized.");
			return 1;
		}
		if (!_inputSystemInitialized) {
			spdlog::critical("CoreController: Cannot initialize game loop, input system not initialized.");
			return 1;
		}

		__raylibInfoLogToDebug = true; // Used by the _raylibLogCallback Method

		// Here is the actual game loop
		spdlog::info("CoreController: Starting game loop...");
		_gameLoopInitialized = true;
		bool __startingGameStateEventAlreadyPushed = false;
		while (!WindowShouldClose()) {
			const DoubleEventBuffer dEventBuffer = _createDoubleEventBuffer();

			if (!__startingGameStateEventAlreadyPushed && _currentGameState.expired()) {
				// When we are not in a GameState, we just trying to get into the startingState
				spdlog::info("CoreController: Pushing switch to startingState into eventBuffer...");
				dEventBuffer.pushEvent(std::make_shared<GameStateSwitchEvent>("CoreController.cpp", startingState));
				__startingGameStateEventAlreadyPushed = true;			
			}

			// Input handling
			if(!_currentGameState.expired() && _currentGameState.lock()->doInput) _inputSystem->checkForInputs(dEventBuffer);

			// Game Logic
			float deltaTime = GetFrameTime();
			_doTick(deltaTime, dEventBuffer);

			// Rendering
			if(_currentScene != nullptr) _sceneRenderer->renderSceneToRendererBuffer(*_currentScene);
			BeginDrawing();	

			if (_currentScene != nullptr) _sceneRenderer->renderBufferToScreen();
			else _sceneRenderer->renderBlank();

			if (_debugMode && _imguiInitialized) _imGUITick(dEventBuffer, deltaTime);
		

			EndDrawing();
		}

		spdlog::info("CoreController: Exiting game loop...");

		if (_currentScene != nullptr) {
			_currentScene->unloadScene();
		}

		_sceneRenderer->unloadRenderTextures();

		spdlog::info("CoreController: Game loop ended, stopping game...");
		_stopGame();

		return 0;
	}

	void CoreController::_processEvents(const DoubleEventBuffer& dEventBuffer)
	{
		for (const auto& event : *dEventBuffer.currentEventBuffer) {

			// Game Key Event handling (Console open/close)
			if (event->type == EventType::KeyInputEvent) {
				auto keyEvent = dynamic_cast<const GameKeyEvent*>(event.get());

				/*spdlog::trace("CoreController: Key event received: keyCode={}, isPressed={}",
					keyEvent->keyCode, keyEvent->isPressed);*/

				if (_consoleInitialized && keyEvent->keyCode == KeyboardKey::KEY_F2) {
					_consoleController->switchConsoleActive();
				}
			}

			// Gui Scene Element Debug Window Switch Event handling
			else if (_imguiInitialized && event->type == EventType::GuiSceneElementDebugWindowSwitch) {
				const GuiSwitchSceneElementDebugWindow* guiElementSwitchEvent = dynamic_cast<const GuiSwitchSceneElementDebugWindow*>(event.get());

				spdlog::trace("CoreController: GuiSwitchSceneElementDebugWindow received: targetShaderName='{}', shaderShouldClose={}",
					guiElementSwitchEvent->targetId, guiElementSwitchEvent->shouldClose);

				// Get Element from current scene by name
				if (_currentScene == nullptr) {
					spdlog::warn("CoreController: Tried to switch gui for shader {}, but the current scene is null", guiElementSwitchEvent->targetId);
					return;
				}

				// Shader Element
				if (guiElementSwitchEvent->element == GuiSwitchSceneElementDebugWindow::DebugWindowElement::Shader) {		
					const auto& currentShaders = _currentScene->getShaderListSorted();
					if (currentShaders.empty()) {
						spdlog::warn("CoreController: Tired to switch gui for shader {}, but there are no shaders registed in scene {}", guiElementSwitchEvent->targetId, _currentScene->sceneName);
						return;
					}

					const auto targetShaderIt = std::find_if(currentShaders.begin(), currentShaders.end(),
						[&guiElementSwitchEvent](const std::weak_ptr<GameShader>& if_shader) {
							return if_shader.lock()->getName() == guiElementSwitchEvent->targetId;
						}
					);

					if (targetShaderIt != currentShaders.end()) {
						if (!guiElementSwitchEvent->shouldClose) {
							_imguiObjectRenderer->addShaderToRenderList(*targetShaderIt);
						}
						else
						{
							_imguiObjectRenderer->removeShaderToRenderList(*targetShaderIt);
						}
					}
					else {
						spdlog::warn("CoreController: Tried to enable gui for shader \"{}\", but it doesnt exist or isn't loaded in the current scene.", guiElementSwitchEvent->targetId);
					}
				}

				// Texture Element
				else if (guiElementSwitchEvent->element == GuiSwitchSceneElementDebugWindow::DebugWindowElement::Texture) {
					const auto& currentTextures = _currentScene->getAllTexturesMap();
					if (currentTextures.empty()) {
						spdlog::warn("CoreController: Tired to switch gui for texture {}, but there are no textures registed in scene {}", guiElementSwitchEvent->targetId, _currentScene->sceneName);
						return;
					}

					const auto targetTextureIt = currentTextures.find(guiElementSwitchEvent->targetId);

					if (targetTextureIt != currentTextures.end()) {
						if (!guiElementSwitchEvent->shouldClose) {
							_imguiObjectRenderer->addTextureToRenderList(targetTextureIt->second);
						}
						else
						{
							_imguiObjectRenderer->removeTextureToRenderList(targetTextureIt->second);
						}
					}
					else {
						spdlog::warn("CoreController: Tried to enable gui for texture \"{}\", but it doesnt exist or isn't loaded in the current scene.", guiElementSwitchEvent->targetId);
					}
				}
			}

			// GameState switch event handling
			else if (event->type == EventType::GameStateSwitch) {
				const GameStateSwitchEvent* gameStateSwitchEvent = dynamic_cast<const GameStateSwitchEvent*>(event.get());

				switchToGameState(gameStateSwitchEvent->targetStateId);
			}

		}
	}

	void CoreController::_generateBasicInfoImGuiWindow(float dt, const DoubleEventBuffer& deb)
	{
		// Simple color switcher for better readability
		ImVec4 switchingColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		static auto colorSwitcherFunc = [&switchingColor] () {
			if(switchingColor.x == 0.7f) {
				switchingColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else {
				switchingColor = ImVec4(0.7f, 0.7f, 0.9f, 1.0f);
			}
		};

		// Helper function to draw a image
		static auto drawImageFunc = [](const GameTexture& tex, int width, int height) {
			ImVec2 imgSize = ImVec2(width, height);
			ImGui::Image((void*)(intptr_t)tex.getTexture().id, imgSize);
		};

		// Rendering the ImGui window
		ImGui::Begin("Graxel Engine Debug Window");

		if (ImGui::Button("Toogle Debug Console")) {
			if (_consoleInitialized) {
				_consoleController->switchConsoleActive();
			}
		}

		ImGui::Text("Current Game State: %s", (_currentGameState.expired()) ? "NOT ASSIGNED" : _currentGameState.lock()->id.c_str());
		ImGui::Text("Current Frame Time: %.2f ms", GetFrameTime() * 1000.0f);
		ImGui::Text("Current Frames per Second: %.2f", 1.0f / dt);
		ImGui::Text("Runtime: %.2f", GetTime());

		ImGui::Separator();

		if (ImGui::CollapsingHeader(std::format("{} Registered Game States", GAME_STATES.size()).c_str())) {
			switchingColor = ImVec4(1.8f, 1.0f, 1.0f, 1.0f);
			for (const auto& state : GAME_STATES) {
				ImGui::TextColored(switchingColor, "GameState ID: \"%s\"\n-> doRender: %s\n-> doInput: %s\n-> Has Update Function: %s\n-> Has onEnterState Function: %s\n-> Has onLeaveState Function: %s",
					state.second->id.c_str(),
					(state.second->doRender) ? "true" : "false",
					(state.second->doInput) ? "true" : "false",
					(state.second->updateFunction != nullptr) ? "true" : "false",
					(state.second->onEnterState != nullptr) ? "true" : "false",
					(state.second->onLeaveState != nullptr) ? "true" : "false"
				);

				ImGui::PushID(std::format("switch state {}", state.second->id).c_str());
				if (ImGui::Button("Swtich to")) {
					deb.pushEvent(std::make_shared<GameStateSwitchEvent>("CoreController.cpp/GeneralImGuiWindow", state.second->id));
				}
				ImGui::PopID();

				ImGui::Separator();
				colorSwitcherFunc();
			}
		}

		ImGui::Separator();
		
		if (ImGui::CollapsingHeader(std::format("{} Registered Scene Definitions", _sceneDefs.size()).c_str())) {
			switchingColor = ImVec4(1.8f, 1.0f, 1.0f, 1.0f);
			for (const auto& scene : _sceneDefs) {
				ImGui::TextColored(switchingColor, "Scene ID: \"%s\"\n-> Scene Name: \"%s\"\n-> Has build function: %s",
					scene.second.sceneId.c_str(),
					scene.second.sceneName.c_str(),
					(scene.second.haveBuildFunction()) ? "true" : "false"
				);

				ImGui::PushID(std::format("build and switch to scene {}", scene.second.sceneId).c_str());
				if(ImGui::Button("Build and Switch to")) {
					buildAndSwitchToScene(scene.second.sceneId);
				}
				ImGui::PopID();

				ImGui::Separator();
				colorSwitcherFunc();
			}
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Current Scene Info")) {
			if (_currentScene != nullptr) {
				ImGui::Text("Scene Name: %s", _currentScene->sceneName.c_str());
				ImGui::Text("Scene ID: %s", _currentScene->sceneId.c_str());
			
				const auto& textures = _currentScene->getAllTexturesMap();
				const auto& shaders = _currentScene->getAllShadersMap();
				const auto& gameObjects = _currentScene->getAllGameObjects();
				if (ImGui::CollapsingHeader(std::format("{} textures loaded", textures.size()).c_str())) {
					switchingColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					for (const std::pair<std::string, std::shared_ptr<GameTexture>>& tex : textures) {
						// Draw the texture
						drawImageFunc(*tex.second, 64, 64);

						// Draw the text
						ImVec2 originalCursorPos = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(80, originalCursorPos.y - 64));
						ImGui::TextColored(switchingColor, "Key: \"%s\"\n-> Name: \"%s\"\n-> Size: %.0fx%.0f",
							tex.first.c_str(), tex.second->name.c_str(), tex.second->getTextureRect().width, tex.second->getTextureRect().height);
						ImGui::SetCursorPos(originalCursorPos);

						ImGui::Separator();

						ImGui::PushID(("open texture " + tex.second->name).c_str());
						if (ImGui::Button("Show")) {
							deb.pushEvent(
								std::make_shared<GuiSwitchSceneElementDebugWindow>(
									GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Texture, tex.first, false)
								)
							);
						}
						ImGui::PopID();
						ImGui::SameLine();
						ImGui::PushID(("close texture " + tex.second->name).c_str());
						if (ImGui::Button("Hide")) {
							deb.pushEvent(
								std::make_shared<GuiSwitchSceneElementDebugWindow>(
									GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Texture, tex.first, true)
								)
							);
						}	
						ImGui::PopID();

						// Seperator and color switch for the next element
						ImGui::Separator();
						colorSwitcherFunc();
					}
				}
				if (ImGui::CollapsingHeader(std::format("{} shaders loaded", shaders.size()).c_str())) {
					switchingColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					for (const std::pair<std::string, std::shared_ptr<GameShader>>& shader : shaders) {
						ImGui::TextColored(switchingColor, "Key: \"%s\"\n-> Name: \"%s\"\n-> Type: %s\n-> Priority: %d\n-> cValuesAmount: %d",
							shader.first.c_str(), shader.second->getName().c_str(), magic_enum::enum_name(shader.second->getType()).data(),
							shader.second->getPriority(), shader.second->getCustomValuesAmount());

						ImGui::PushID(std::string("open shader " + shader.second->getName()).c_str());
						if (ImGui::Button("Show")) {
							deb.pushEvent(
								std::make_shared<GuiSwitchSceneElementDebugWindow>(
									GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Shader, shader.second->getName(), false)
								)
							);
						}
						ImGui::PopID();
						ImGui::SameLine();
						ImGui::PushID(std::string("close shader " + shader.second->getName()).c_str());
						if (ImGui::Button("Hide")) {
							deb.pushEvent(
								std::make_shared<GuiSwitchSceneElementDebugWindow>(
									GuiSwitchSceneElementDebugWindow("ConsoleController.cpp", GuiSwitchSceneElementDebugWindow::DebugWindowElement::Shader, shader.second->getName(), true)
								)
							);
						}
						ImGui::PopID();

						ImGui::Separator();
						colorSwitcherFunc();
					}
				}
				if (ImGui::CollapsingHeader(std::format("{} gameObjects initiated", gameObjects.size()).c_str())) {
					switchingColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					for (const std::shared_ptr<GameObject>& go : gameObjects) {
						drawImageFunc(*go->texture.lock(), 70, 70);

						ImVec2 originalCursorPos = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(86, originalCursorPos.y - 70));
						ImGui::TextColored(switchingColor, "Name: \"%s\"\n-> Pos: %s\n-> Size: %s\n-> DrawOrder: %d\n-> Texture: \"%s\"",
							go->name.c_str(), go->position.string().c_str(), go->size.string().c_str(), go->drawOrder, go->texture.lock()->name.c_str());
						ImGui::SetCursorPos(originalCursorPos);

						ImGui::Separator();
						colorSwitcherFunc();
					}
				}
			}
			else ImGui::Text("No Scene loaded.");
		}

		ImGui::Separator();
		ImGui::Separator();

		ImGui::TextDisabled("Graxl Engine %s", info::GRAXL_ENGINE_VERSION);
		ImGui::TextDisabled("Developed by BlackBlocky");

		ImGui::End();
	}

	// Swaps the current and next event buffer, and clears all processed events
	// Also returns a DoubleEventBuffer that contains the current and next event buffer
	DoubleEventBuffer CoreController::_createDoubleEventBuffer()
	{
		// Manage event buffers
		const std::vector<std::shared_ptr<const GameEvent>>* currentEventBuffer = nullptr;
		std::vector<std::shared_ptr<const GameEvent>>* nextEventBuffer = nullptr;
		if (!_isSecondEventCycle) {
			currentEventBuffer = &_eventBufferA;

			nextEventBuffer = &_eventBufferB;
			nextEventBuffer->clear();
		}
		else {
			currentEventBuffer = &_eventBufferB;

			nextEventBuffer = &_eventBufferA;
			nextEventBuffer->clear();
		}
		_isSecondEventCycle = !_isSecondEventCycle;

		return DoubleEventBuffer(currentEventBuffer, nextEventBuffer);
	}

	void CoreController::_doTick(float dt, const DoubleEventBuffer& dEventBuffer)
	{
		// Call the CoreController event processor to process events before the actual game tick
		_processEvents(dEventBuffer);

		// Call the tickupdate on every system that active in the current game state
		if (_currentGameState.expired() || _currentGameState.lock()->updateFunction == nullptr) return;
		_currentGameState.lock()->updateFunction(*this, dt);
	}

	void CoreController::_imGUITick(const DoubleEventBuffer& doubleEventBuffer, float dt)
	{
		rlImGuiBegin();

		_generateBasicInfoImGuiWindow(dt, doubleEventBuffer);

		// Console
		if (_consoleInitialized && _consoleController->isConsoleActive()) {
			_consoleController->consoleImGuiUpdate(doubleEventBuffer);
		}

		_imguiObjectRenderer->tick();

		rlImGuiEnd();
	}

	void CoreController::switchToGameState(const std::string stateId)
	{
		if (GAME_STATES.find(stateId) == GAME_STATES.end()) {
			spdlog::error("CoreController: Tried switching to GameState \"{}\", but it doesnt exist a GameState with that id!", stateId);
			spdlog::error("... GameState wont switch!");
			return;
		}

		spdlog::debug("CoreController: Switching to Game State {}...", stateId);
		std::shared_ptr<const GameState> oldGameStatePtr = _currentGameState.lock();
		if (oldGameStatePtr != nullptr) {
			if (oldGameStatePtr->onLeaveState != nullptr) {
				spdlog::trace("CoreController: Calling onLeaveState for the previous state: {}", oldGameStatePtr->id);
				oldGameStatePtr->onLeaveState(*this, *oldGameStatePtr);
			}
			else spdlog::trace("CoreController: Previous GameState \"{}\" has no onLeaveState function, so nothing will be called.", oldGameStatePtr->id);
		}
		else spdlog::trace("CoreController: No previous GameState set, so no onLeaveState will be called.");
		
		_currentGameState = GAME_STATES[stateId];

		spdlog::trace("CoreController: Calling onEnterState for the new state: {}", stateId);
		std::shared_ptr<const GameState> newGameStatePtr = _currentGameState.lock();
		if (newGameStatePtr->onEnterState != nullptr) {
			newGameStatePtr->onEnterState(*this, *newGameStatePtr);
			spdlog::debug("CoreController: GameState changed to {}", stateId);
		}
		else spdlog::trace("CoreController: New GameState \"{}\" has no onEnterState function, so nothing will be called.", stateId);
	}

	void CoreController::_switchToScene(Scene&& scene)
	{
		spdlog::debug("CoreController: Switching scenes: NewID:\"{}\", OldID:\"{}\"", ((_currentScene != nullptr) ? _currentScene->sceneId : "NONE"), scene.sceneId);

		// Deload the current scene if there is one
		if (_currentScene != nullptr) {
			spdlog::info("CoreController: Unloading current scene \"{}\" (id:{})...", _currentScene->sceneName, _currentScene->sceneId);
			_currentScene->unloadScene();
		}

		spdlog::info("CoreController: Switching to new scene \"{}\" (id:{})...", scene.sceneName, scene.sceneId);
		_currentScene = std::make_shared<Scene>(std::move(scene));

		// Note: We dont have to worry about the old scene, because we using shared_ptr. So no mempory leaks should happen :P
 	}

	void CoreController::_stopGame()
	{
		if (_imguiInitialized) {
			spdlog::info("CoreController: Stopping ImGUI...");
			rlImGuiEnd(); // ends the ImGui content mode
			rlImGuiShutdown(); // cleans up ImGui
		}

		spdlog::info("CoreController: Stopping game...");
		CloseWindow();
	}

};