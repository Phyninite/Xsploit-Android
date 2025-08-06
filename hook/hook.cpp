#include "hook.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

bool hook_mgr::hook(uintptr_t target_address, void* replacement) {
    const auto page_size = getpagesize();
    const auto page_start = target_address & ~(page_size - 1);
    
    if (mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    
    const auto relative_offset = reinterpret_cast<uintptr_t>(replacement) - target_address - 4;
    const uint32_t branch_instruction = 0x14000000 | ((relative_offset >> 2) & 0x3FFFFFF);
    
    *reinterpret_cast<uint32_t*>(target_address) = branch_instruction;
    
    __builtin___clear_cache(reinterpret_cast<char*>(target_address), 
                           reinterpret_cast<char*>(target_address + 4));
    
    return true;
}
