#pragma once

#include "graxlEngine/event/GameEvent.h"

namespace graxl {

	class GuiSwitchSceneElementDebugWindow : public GameEvent {
	public:
		enum class DebugWindowElement {
			Shader,
			Texture
		};

		GuiSwitchSceneElementDebugWindow(const std::string origin, const DebugWindowElement _element, const std::string _targetId, const bool _shouldClose)
			: GameEvent("guiSceneElementDebugWindowSwitch", origin, EventType::GuiSceneElementDebugWindowSwitch), element(_element), targetId(_targetId), shouldClose(_shouldClose) {
		};

		const DebugWindowElement element;
		const std::string targetId;
		const bool shouldClose;
	};

};