#pragma once

#include <vector>
#include <memory>
#include <utility>

#include <graxlEngine/event/GameEvent.h>

#include <spdlog/spdlog.h>

namespace graxl {

class DoubleEventBuffer
{
public:
	// The pointers are always alive as long as the GameController is alive, so its impossible they refer to a nullptr
	DoubleEventBuffer(
		std::vector<std::shared_ptr<const GameEvent>>* __currentEventBuffer,
		std::vector<std::shared_ptr<const GameEvent>>* __nextEventBuffer
	) : _currentEventBuffer(__currentEventBuffer), _nextEventBuffer(__nextEventBuffer) {
	};

	void pushEvent(std::shared_ptr<const GameEvent> event) const {
		spdlog::trace("Pushing event \"{}\" in to event buffer... Origin: \"\{}\"", event->eventName, event->origin);
		_nextEventBuffer->push_back(event);
	};

	const std::vector<std::shared_ptr<const GameEvent>>& getCurrentEventBuffer() const {
		return *_currentEventBuffer;
	}

	// Updates the current and next Buffer. Should only be called by the Engine Game Loop!
	void updateBuffers(std::vector<std::shared_ptr<const GameEvent>>* __current, std::vector<std::shared_ptr<const GameEvent>>* __next) {
		_currentEventBuffer = __current;
		_nextEventBuffer = __next;
	};

	// Swaps next and current buffer#
	// The new current Buffer will be cleared
	void swapAndClearBuffers() {
		auto oldCurrentPtr = _currentEventBuffer;

		_currentEventBuffer = _nextEventBuffer;
		_nextEventBuffer = oldCurrentPtr;
		
		// Clearing the nexr-event-bzffer, because its was orignally the old current event buffer. Which means it contains old events.
		_nextEventBuffer->clear();
	}

private:
	// Promise that the pointers wotn be invalid until the Object is destroyed

	std::vector<std::shared_ptr<const GameEvent>>* _currentEventBuffer;
	std::vector<std::shared_ptr<const GameEvent>>* _nextEventBuffer;
};

};