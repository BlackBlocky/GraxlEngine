#pragma once

#include <string>

namespace graxl {

enum class EventType {
	KeyInputEvent, // GameKeyEvent
	
	GuiShaderSwitchEvent, // GuiShaderSwitchEvent

	GameStateSwitch, // Event to switch Game States
};

class GameEvent {
public:
	GameEvent(const std::string __name, const std::string __origin, const EventType __type) : eventName(__name), origin(__origin), type(__type) {}
	virtual ~GameEvent() = default;
	const std::string eventName;
	const std::string origin;
	const EventType type;
};

};