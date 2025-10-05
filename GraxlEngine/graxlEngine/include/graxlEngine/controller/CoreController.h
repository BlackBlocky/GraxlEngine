#pragma once

#include <graxlEngine/controller/MenuController.h>
#include <graxlEngine/controller/ConsoleController.h>
#include <graxlEngine/InputSystem.h>
#include <graxlEngine/ui/ImGUIObjectRenderer.h>

#include <graxlEngine/event/GameEvent.h>
#include <graxlEngine/event/GameKeyEvent.h>
#include <graxlEngine/event/DoubleEventBuffer.h>

#include <graxlEngine/render/SceneRenderer.h>

#include <optional>
#include <string>
#include <functional>
#include <memory>

namespace graxl {

	constexpr const char* ASSETS_PATH = "C:\\Users\\eliah\\coding\\repos\\vstudio\\cmake\\GrashCpp\\assets\\";

class CoreController {
public:
	// Init Functions
	int init(bool debugMode, int framerate);

	int loadGameConfig();
	int loadGlobalAssets();
	int initWindow(int framerate);
	int defineGameStates();
	int initInputSystem();
	int startImGUI();
	int initConsole();

	int initGameLoop(std::string startingState);

	// Config Functions
	struct GameState {
		std::string id;

		bool doRender;
		bool doInput;

		std::function<void(float)> updateFunction;
	};
	void addGameState(GameState&& newGameState);

	// const getter
	const std::unordered_map<std::string, std::shared_ptr<const GameState>> getDefinedGameStates() const {
		return this->GAME_STATES;
	}

private:
	// Gamestates
	std::unordered_map<std::string, std::shared_ptr<const GameState>> GAME_STATES;
	std::weak_ptr<const GameState> _currentGameState;

	// Status booleans
	bool _gloablAssetsLoaded = false;
	bool _windowInitialized = false;
	bool _inputSystemInitialized = false;
	bool _imguiInitialized = false;
	bool _consoleInitialized = false;
	bool _gameLoopInitialized = false;

	// Other controll classes
	MenuController _menuController;
	SceneRenderer _sceneRenderer;
	std::optional<InputSystem> _inputSystem;
	std::optional<ConsoleController> _consoleController;
	std::optional<ImGUIObjectRenderer> _imguiObjectRenderer;

	// Data
	std::optional<Scene> _currentScene;

	bool _debugMode = false;

	DoubleEventBuffer _createDoubleEventBuffer();
	std::vector<std::shared_ptr<const GameEvent>> _eventBufferA;
	std::vector<std::shared_ptr<const GameEvent>> _eventBufferB;
	bool _isSecondEventCycle = false;

	// Intern methods
	void _doTick(float dt, const DoubleEventBuffer& dEventBuffer);
	void _imGUITick(const DoubleEventBuffer& doubleEventBuffer);
	void _stopGame();

	void _processEvents(const DoubleEventBuffer& dEventBuffer);

	static void _raylibLogCallback(int msgType, const char* text, va_list args);

	void switchToGameState(const std::string stateId);

};

};