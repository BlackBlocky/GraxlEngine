#pragma once

#include <graxlEngine/controller/ConsoleController.h>
#include <graxlEngine/InputSystem.h>
#include <graxlEngine/ui/ImGUIObjectRenderer.h>
#include <graxlEngine/scene/SceneDef.h>

#include <graxlEngine/event/GameEvent.h>
#include <graxlEngine/event/GameKeyEvent.h>
#include <graxlEngine/event/DoubleEventBuffer.h>

#include <graxlEngine/render/SceneRenderer.h>

#include <optional>
#include <string>
#include <functional>
#include <memory>

namespace graxl {

class CoreController {
public:
	// Init Functions
	int init(const std::string& windowTitle, Vector2 aspectRatio, int baseWindowWidthPixels, int pixelsPerUnit, int framerate, bool debugMode);
	int initGameLoop(std::string startingState);

	// Config Functions
	struct GameState {
		std::string id;

		bool doRender;
		bool doInput;

		std::function<void(CoreController&, float)> updateFunction;

		std::function<void(CoreController&, const GameState&)> onEnterState;
		std::function<void(CoreController&, const GameState&)> onLeaveState;
	};
	void addGameState(GameState&& newGameState);

	// Unloads the current scene with all its contents if there is one, and creates an empty scene with the given name
	std::weak_ptr<Scene> createEmptyScene(const std::string sceneName);
	// Creates a new loader for a scene
	void createSceneDefinition(const std::string& id, const std::string& name, std::function<void(Scene& scene)> buildFunction);
	// Switches to a scene by the scene id. Scene have to be added by the createSceneDefinition(...) function
	std::weak_ptr<Scene> buildAndSwitchToScene(const std::string& id);


	const std::weak_ptr<Scene> getCurrentSceneWeakPtr() const { return _currentScene; }

	// const getter
	const std::unordered_map<std::string, std::shared_ptr<const GameState>> getDefinedGameStates() const {
		return this->GAME_STATES;
	}

private:
	// Gamestates
	std::unordered_map<std::string, std::shared_ptr<const GameState>> GAME_STATES;
	std::unordered_map<std::string, SceneDef> _sceneDefs;
	std::weak_ptr<const GameState> _currentGameState;

	// Status booleans
	bool _windowInitialized = false;
	bool _inputSystemInitialized = false;
	bool _imguiInitialized = false;
	bool _consoleInitialized = false;
	bool _gameLoopInitialized = false;

	// Other controll classes
	std::optional<SceneRenderer> _sceneRenderer;
	std::optional<InputSystem> _inputSystem;
	std::optional<ConsoleController> _consoleController;
	std::optional<ImGUIObjectRenderer> _imguiObjectRenderer;

	// Data
	std::shared_ptr<Scene> _currentScene = nullptr;

	bool _debugMode = false;

	DoubleEventBuffer _createDoubleEventBuffer();
	std::vector<std::shared_ptr<const GameEvent>> _eventBufferA;
	std::vector<std::shared_ptr<const GameEvent>> _eventBufferB;
	bool _isSecondEventCycle = false;

	// Init Methods

	int _initWindow(int framerate, const std::string& title, Vector2 res);
	int _initInputSystem();
	int _startImGUI();
	int _initConsole();

	// Intern methods
	void _doTick(float dt, const DoubleEventBuffer& dEventBuffer);
	void _imGUITick(const DoubleEventBuffer& doubleEventBuffer);
	void _stopGame();

	void _processEvents(const DoubleEventBuffer& dEventBuffer);

	static void _raylibLogCallback(int msgType, const char* text, va_list args);

	void switchToGameState(const std::string stateId);

	void _switchToScene(Scene&& scene);

};

};