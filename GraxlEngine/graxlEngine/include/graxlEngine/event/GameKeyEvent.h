#pragma once

#include "graxlEngine/event/GameEvent.h"

namespace graxl {

class GameKeyEvent : public GameEvent {
public:
	GameKeyEvent(const std::string origin, const int _keyCode, const bool _isPressed)
		: GameEvent("keyInputEvent", origin, EventType::KeyInputEvent), keyCode(_keyCode), isPressed(_isPressed) {
	}

	const int keyCode; // contains a raylib KeyboardKey enum value
	const bool isPressed; // indicates if the key was pressed or released
};

};