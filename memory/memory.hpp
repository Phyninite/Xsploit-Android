#pragma once
#include <dlfcn.h>
#include <link.h>
#include <cstdint>

class mem_mgr {
private:
    static uintptr_t cached_base;
    
public:
    static uintptr_t get_lib_base(const char* lib_name) noexcept;
    static uintptr_t rebase(const uintptr_t rva) noexcept;
};
