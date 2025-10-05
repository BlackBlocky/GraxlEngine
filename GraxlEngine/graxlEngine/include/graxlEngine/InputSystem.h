#pragma once

#include "graxlEngine/event/GameKeyEvent.h"
#include "graxlEngine/event/DoubleEventBuffer.h"

#include <functional>
#include <memory>

namespace graxl {

class InputSystem {
public:
	void checkForInputs(const DoubleEventBuffer& dEventBuffer);
	std::string getPressedKeyName(int keyboardKey);
};

};