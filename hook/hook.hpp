#pragma once
#include <cstdint>

class hook_mgr {
public:
    static bool hook(uintptr_t target_address, void* replacement);
};
