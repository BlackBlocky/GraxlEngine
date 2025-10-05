#pragma once

#include <raylib.h>
#include <spdlog/spdlog.h>

#include <graxlEngine/event/GameEvent.h>
#include <graxlEngine/event/DoubleEventBuffer.h>
#include <graxlEngine/render/GameTexture.h>

namespace graxl {

class GameObject {
public:
	GameObject(GameTexture& __texture)
		: texture(__texture), position({ 0, 0 }), size({ 1, 1 }), drawOrder(0) {};

	Vector2 position;
	Vector2 size;

	GameTexture& texture;

	// Higher = Foreground
	int drawOrder;

	virtual void update(float& dt, float& timeProgress, DoubleEventBuffer& dEventBuffer) {
		spdlog::warn("GameObject: GameObject::update called without a implementation");
	};
};

}