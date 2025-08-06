#include "memory.hpp"

std::uintptr_t mem_mgr::cached_base = 0;

std::uintptr_t mem_mgr::get_lib_base(const char* lib_name) noexcept {
    const auto handle = dlopen(lib_name, RTLD_LAZY | RTLD_NOLOAD);
    link_map* map;
    dlinfo(handle, RTLD_DI_LINKMAP, &map);
    dlclose(handle);
    cached_base = map->l_addr;
    return cached_base;
}

std::uintptr_t mem_mgr::rebase(const std::uintptr_t rva) noexcept {
    return cached_base + rva;
}
