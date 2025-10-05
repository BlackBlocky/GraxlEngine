#pragma once

namespace graxl {

class AnyAsset {
public:
	virtual ~AnyAsset() = default;
};

class SpriteAsset : public AnyAsset {
public:
	SpriteAsset();
};

};