#pragma once

#include "model_manager.h"

struct ObjectInstance {
    std::vector<ModelInstance> instances;
    Transform t;
};