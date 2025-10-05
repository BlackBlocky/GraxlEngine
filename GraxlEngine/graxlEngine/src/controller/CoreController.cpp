#include "graxlEngine/controller/CoreController.h"

#include <format>

#include <raylib.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <rlImGui.h>

#include <graxlEngine/render/GameShader.h>
#include <graxlEngine/event/GuiShaderSwitchEvent.h>
#include <graxlEngine/event/GameStateSwitchEvent.h>

namespace graxl {

	int CoreController::init(bool debugMode, int framerate) {
		_debugMode = debugMode;

		if (this->loadGlobalAssets() != 0) return 1;
		if (this->initWindow(framerate) != 0) return 1;
		if (this->initInputSystem() != 0) return 1;
		if (this->startImGUI() != 0) return 1;
		if (this->initConsole() != 0) return 1;

		return 0;
	}

	int CoreController::loadGlobalAssets()
	{

		_gloablAssetsLoaded = true;
		spdlog::info("CoreController: Global assets loaded successfully.");
		return 0;
	}


	int CoreController::initWindow(int framerate)
	{
		// Move Raylib logging to spdlog logging
		SetTraceLogCallback(_raylibLogCallback);

		spdlog::info("CoreController: Initializing window...");

		SetConfigFlags(FLAG_WINDOW_RESIZABLE);

		InitWindow(1920, 1080, "Grash Cpp");
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

	int CoreController::initInputSystem()
	{
		spdlog::info("CoreController: Initializing input system...");

		_inputSystem = InputSystem();

		_inputSystemInitialized = true;
		spdlog::info("CoreController: Input system initialized successfully.");
		return 0;
	}

	int CoreController::startImGUI()
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

	int CoreController::initConsole()
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

	int CoreController::initGameLoop(std::string startingState)
	{
		// Make sure everything is settep up correctly before starting the game loop
		if (!_gloablAssetsLoaded) {
			spdlog::critical("CoreController: Cannot initialize game loop, global assets not loaded.");
			return 1;
		}
		if (!_windowInitialized) {
			spdlog::critical("CoreController: Cannot initialize game loop, window not initialized.");
			return 1;
		}
		if (!_inputSystemInitialized) {
			spdlog::critical("CoreController: Cannot initialize game loop, input system not initialized.");
			return 1;
		}

		__raylibInfoLogToDebug = true; // Used by the _raylibLogCallback Method

		// TESTING ----
		GameTexture tex = GameTexture(LoadTexture((std::string(ASSETS_PATH) + "textures/MagnetSnakeDown.png").c_str()), "Test-GameTexture-Player");
		GameTexture tex2 = GameTexture(LoadTexture((std::string(ASSETS_PATH) + "textures/SpikeDown.png").c_str()), "Test-GameTexture-Spike");

		SetTextureFilter(tex.getTexture(), TEXTURE_FILTER_POINT);
		SetTextureFilter(tex2.getTexture(), TEXTURE_FILTER_POINT);

		GameObject go = GameObject(tex);
		GameObject go2 = GameObject(tex2);
		go2.position = { 3, 3 };

		go.size = { 1, 1 };

		_currentScene.emplace("Test Scene");
		_currentScene->gameObjects.push_back(std::make_unique<GameObject>(std::move(go)));
		_currentScene->gameObjects.push_back(std::make_unique<GameObject>(std::move(go2)));

		GameShader sShader = GameShader("s01", GameShaderType::GLSL, 500, 6);
		sShader.loadShaderFromFile(std::string(ASSETS_PATH) + "testShader.glsl");

		auto test_shaderData = sShader.getShaderUniformDataPtr();

		_currentScene->addShader(std::move(sShader));


		GameShader sShader2 = GameShader("s02", GameShaderType::GLSL, 1000, 1);
		sShader2.loadShaderFromFile(std::string(ASSETS_PATH) + "testShader2.glsl");

		std::shared_ptr<GameShader::ShaderUniformData> test_shaderData2 = sShader2.getShaderUniformDataPtr();
		_currentScene->addShader(std::move(sShader2));
	
		test_shaderData2->intensity = 1.0f;
		test_shaderData2->customValues[0] = 1000.0f;

	

		//if (_imguiInitialized) _imguiObjectRenderer->addShaderToRenderList(_currentScene->getShaders()[0]);

		//if (_imguiInitialized) {
		//	// Remove constness by making a local non-const shared_ptr
		//	auto shaderPtr = coolScene.getShaders()[0];
		//	_imguiObjectRenderer->addShaderToRenderList(shaderPtr);
		//}

		// Adding a test Image
		GameTexture testImgTexture = GameTexture(LoadTexture("C:\\Users\\eliah\\Downloads\\o-FORESTS-facebook-2994683020.jpg"), "test-img-tex");
		GameObject testImg = GameObject(testImgTexture);
		testImg.size = {20, 13};
		testImg.drawOrder = -1000;
		_currentScene->gameObjects.push_back(std::make_unique<GameObject>(std::move(testImg)));

		// TESTING ----

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
			_doTick(GetFrameTime(), dEventBuffer);

			// Rendering
			_sceneRenderer.renderSceneToRendererBuffer(*_currentScene);
			_currentScene->getShaders()[0]->getShaderUniformDataPtr()->globalTime = static_cast<float>(GetTime()); // TESTING
			BeginDrawing();	

			_sceneRenderer.renderBufferToScreen();
			if (_debugMode && _imguiInitialized) _imGUITick(dEventBuffer);
		

			EndDrawing();
		}

		spdlog::info("CoreController: Exiting game loop...");

		tex.unloadGameTexture(); // TESTING
		tex2.unloadGameTexture();
		testImgTexture.unloadGameTexture();

		//sShader.unloadGameShader();
		_currentScene->unloadScene();

		_sceneRenderer.unloadRenderTextures();

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

			// Gui Shader Switch Event handling
			else if (_imguiInitialized && event->type == EventType::GuiShaderSwitchEvent) {
				const GuiShaderSwitchEvent* guiShaderSwitchEvent = dynamic_cast<const GuiShaderSwitchEvent*>(event.get());

				spdlog::trace("CoreController: GuiShaderSwitchEvent received: targetShaderName='{}', shaderShouldClose={}",
					guiShaderSwitchEvent->targetShaderName, guiShaderSwitchEvent->shaderShouldClose);

				// Get shader from current scene by name
				const auto& currentShaders = _currentScene->getShaders();
				const auto targetShaderIt = std::find_if(currentShaders.begin(), currentShaders.end(),
					[&guiShaderSwitchEvent](const std::shared_ptr<GameShader>& if_shader) {
						return if_shader->getName() == guiShaderSwitchEvent->targetShaderName;
					}
				);

				if (targetShaderIt != currentShaders.end()) {
					if (!guiShaderSwitchEvent->shaderShouldClose) {
						_imguiObjectRenderer->addShaderToRenderList(*targetShaderIt);
					}
					else
					{
						_imguiObjectRenderer->removeShaderToRenderList(*targetShaderIt);
					}
				}
				else {
					spdlog::warn("CoreController: Tried to enable gui for shader \"{}\", but it doesnt exist or isn't loaded in the current scene.", guiShaderSwitchEvent->targetShaderName);
				}
			}

			// GameState switch event handling
			else if (event->type == EventType::GameStateSwitch) {
				const GameStateSwitchEvent* gameStateSwitchEvent = dynamic_cast<const GameStateSwitchEvent*>(event.get());

				switchToGameState(gameStateSwitchEvent->targetStateId);
			}

		}
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
		_currentGameState.lock()->updateFunction(dt);
	}

	void CoreController::_imGUITick(const DoubleEventBuffer& doubleEventBuffer)
	{
		rlImGuiBegin();

		ImGui::Begin("Grash yayy");
		ImGui::Text("This is a cool imGui window.");
		if (ImGui::Button("Write something in the console")) {
			spdlog::debug("CoreController: Button clicked in ImGui window.");
		}

		if (ImGui::CollapsingHeader("More")) {
			ImGui::Text("Current Game State: %s", (_currentGameState.expired()) ? "NOT ASSIGNED" : _currentGameState.lock()->id.c_str());
			ImGui::Text("Current Frame Time: %.2f ms", GetFrameTime() * 1000.0f);
		}

		ImGui::End();

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
			spdlog::trace("CoreController: Calling onLeaveState for the previous state: {}", oldGameStatePtr->id);
			oldGameStatePtr->onLeaveState(*oldGameStatePtr);
		}
		else spdlog::trace("CoreController: No previous GameState set, so no onLeaveState will be called.");
		
		_currentGameState = GAME_STATES[stateId];

		spdlog::trace("CoreController: Calling onEnterState for the new state: {}", stateId);
		std::shared_ptr<const GameState> newGameStatePtr = _currentGameState.lock();
		newGameStatePtr->onEnterState(*newGameStatePtr);
		spdlog::debug("CoreController: GameState changed to {}", stateId);
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