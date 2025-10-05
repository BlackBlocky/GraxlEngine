#pragma once

#include "graxlEngine/scene/Scene.h"   

#include <string>

namespace graxl {

class JSONLoader {
public:
    JSONLoader() = default;
    ~JSONLoader() = default;

    static Scene loadScene(std::string path);
    static void loadSettings();

private:

};

};
