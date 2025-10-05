#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "graxlEngine/event/DoubleEventBuffer.h"

namespace graxl {

	class CoreController;

	class ConsoleController {
	public:
		ConsoleController(CoreController& coreControllerRef);

		void switchConsoleActive() {
			_consoleActive = !_consoleActive;
		}
		bool isConsoleActive() {
			return _consoleActive;
		}

		void consoleImGuiUpdate(const DoubleEventBuffer& doubleEventBuffer);

	private:
		void processCommand(std::string command, const DoubleEventBuffer& doubleEventBuffer);

		//std::function<void(std::shared_ptr<const GameEvent>)> _inputEventCallback;

		bool _consoleActive = false;

		// Intern
		std::vector<std::string> _consoleLogs;
		char _inputBuffer[256] = "";
	
	
		const CoreController& _coreControllerRef; // This reference will life longer than this class (Promise)
	};

};