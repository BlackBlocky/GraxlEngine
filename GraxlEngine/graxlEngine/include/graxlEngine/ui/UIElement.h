#pragma once

#include <raylib.h>
#include <string>

#include <graxlEngine/GVec.h>

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

	UIElement(UIElementType type, const GVec2i position, const GVec2i size, const std::string id, const std::string textValue, bool boolValue)
		: type(type), position(position), size(size), id(id), textValue(textValue), boolValue(boolValue) {
	}

	UIElementType type;
	GVec2i position;
	GVec2i size;
	std::string id;
	std::string textValue;
	bool boolValue;
};

};