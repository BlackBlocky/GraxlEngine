#pragma once

#include <raylib.h>
#include <spdlog/spdlog.h>

#include <graxlEngine/event/GameEvent.h>
#include <graxlEngine/event/DoubleEventBuffer.h>
#include <graxlEngine/render/GameTexture.h>

#include <memory>

namespace graxl {

class GameObject {
public:
	GameObject(std::weak_ptr<GameTexture> __texture, const std::string& __name)
		: texture(__texture), position({ 0, 0 }), size({ 1, 1 }), drawOrder(0), name(__name) {};

	std::string name;

	Vector2 position;
	Vector2 size;

	// Setter
	inline GameObject& setName(const std::string& v) { name = v; return *this; };
	inline GameObject& setPosition(const Vector2& v) { position = v; return *this; };
	inline GameObject& setSize(const Vector2& v) { size = v; return *this; };
	inline GameObject& setDrawOrder(const int v) { drawOrder = v; return *this; };
	inline GameObject& setTexture(std::weak_ptr<GameTexture> v) { texture = v; return *this; };

	// Should ONLY be a texture from a scene the GameObject itself also exists. Otherwise there probably be a looooot of trouble
	std::weak_ptr<GameTexture> texture;

	// Higher = Foreground
	int drawOrder;

	virtual void update(float& dt, float& timeProgress, DoubleEventBuffer& dEventBuffer) {
		spdlog::warn("GameObject: GameObject::update called without a implementation");
	};
};

}