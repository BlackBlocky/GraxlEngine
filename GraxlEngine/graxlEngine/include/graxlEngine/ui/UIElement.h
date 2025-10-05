#pragma once

#include <raylib.h>
#include <string>

namespace graxl {

class UIElement {
public:
	const enum class UIElementType {
		Button,
		Label,
		TextField,
		Image,
		Checkbox,
		Slider,
		Dropdown
	};

	UIElement(UIElementType type, const Vector2 position, const Vector2 size, const std::string id, const std::string textValue, bool boolValue)
		: type(type), position(position), size(size), id(id), textValue(textValue), boolValue(boolValue) {
	}

	UIElementType type;
	Vector2 position;
	Vector2 size;
	std::string id;
	std::string textValue;
	bool boolValue;
};

};