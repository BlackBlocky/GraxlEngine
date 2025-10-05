#pragma once

#include <vector>
#include <memory>

#include <graxlEngine/event/GameEvent.h>

#include <spdlog/spdlog.h>

namespace graxl {

struct DoubleEventBuffer
{
public:
	// The pointers are always alive as long as the GameController is alive, so its impossible they refer to a nullptr
	DoubleEventBuffer(
		const std::vector<std::shared_ptr<const GameEvent>>* const __currentEventBuffer,
		std::vector<std::shared_ptr<const GameEvent>>* const __nextEventBuffer
	) : currentEventBuffer(__currentEventBuffer), nextEventBuffer(__nextEventBuffer) {
	}

	void pushEvent(std::shared_ptr<const GameEvent> event) const {
		if (nextEventBuffer != nullptr) {
			nextEventBuffer->push_back(event);

			spdlog::trace("DoubleEventBuffer: Pushed event to next event buffer: {}. Source: {}", event->eventName, event->origin);
		}
	}

	const std::vector<std::shared_ptr<const GameEvent>>* const currentEventBuffer;

private:
	std::vector<std::shared_ptr<const GameEvent>>* const nextEventBuffer;
};

};