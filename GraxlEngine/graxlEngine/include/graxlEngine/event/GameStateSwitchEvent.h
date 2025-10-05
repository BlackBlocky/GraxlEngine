#pragma once

#include "graxlEngine/event/GameEvent.h"

namespace graxl {

	class GameStateSwitchEvent : public GameEvent {
	public:
		GameStateSwitchEvent(const std::string origin, const std::string _targetStateId)
			: GameEvent("GameStateSwitch", origin, EventType::GameStateSwitch), targetStateId(_targetStateId) {
		}

		const std::string targetStateId;
	};

}