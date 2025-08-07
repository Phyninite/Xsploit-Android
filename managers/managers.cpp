#include "managers.hpp"
#include "../memory/memory.hpp"


extern class rbx_mgr {
public:
    static void initialize();
    static void print(const char* message);
};

void managers::initialize() {
    mem_mgr::get_lib_base("libroblox.so");
    rbx_mgr::initialize();
}

__attribute__((constructor))
void on_load() {
    managers::initialize();
}
