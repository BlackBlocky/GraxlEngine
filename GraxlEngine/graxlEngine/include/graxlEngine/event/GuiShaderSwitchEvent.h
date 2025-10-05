#pragma once

#include "graxlEngine/event/GameEvent.h"

namespace graxl {

	class GuiShaderSwitchEvent : public GameEvent {
	public:
		GuiShaderSwitchEvent(const std::string origin, const std::string _targetShaderName, const bool _shaderShouldClose)
			: GameEvent("guiShaderSwitch", origin, EventType::GuiShaderSwitchEvent), shaderShouldClose(_shaderShouldClose), targetShaderName(_targetShaderName) {
		};

		const std::string targetShaderName;
		const bool shaderShouldClose;
	};

};