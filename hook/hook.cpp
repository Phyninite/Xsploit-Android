#include "hook.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <unordered_map>
#include <cstdint>

static std::unordered_map<uintptr_t, void*> trampolines;

#ifdef __aarch64__
#define NUM_INSTRUCTIONS_TO_OVERWRITE 5
#define INSTRUCTION_SIZE 4
#else
#define INSTRUCTION_SIZE_ARM 4
#define INSTRUCTION_SIZE_THUMB 2
#define HOOK_JUMP_SIZE_ARM 8
#define HOOK_JUMP_SIZE_THUMB 8
#endif

#define IS_THUMB_MODE(addr) ((addr) & 1)

bool hook_mgr::hook(uintptr_t target_address, void* replacement) {
    const auto actual_target_address = target_address & ~1;
    const auto page_size = getpagesize();
    const auto page_start = actual_target_address & ~(page_size - 1);
    
#ifdef __aarch64__
    const auto overwrite_size = NUM_INSTRUCTIONS_TO_OVERWRITE * INSTRUCTION_SIZE;
#else
    const auto is_thumb_mode = IS_THUMB_MODE(target_address);
    const auto overwrite_size = is_thumb_mode ? HOOK_JUMP_SIZE_THUMB : HOOK_JUMP_SIZE_ARM;
#endif
    
    if (mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    
    const auto trampoline = mmap(nullptr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trampoline == MAP_FAILED) {
        return false;
    }
    
    memcpy(trampoline, reinterpret_cast<void*>(actual_target_address), overwrite_size);
    
#ifdef __aarch64__
    const auto jump_back_address = actual_target_address + overwrite_size;
    const auto tramp_ptr = static_cast<uint32_t*>(trampoline);
    const auto hook_ptr = reinterpret_cast<uint32_t*>(actual_target_address);

    const auto tramp_offset = (jump_back_address - reinterpret_cast<uintptr_t>(&tramp_ptr[NUM_INSTRUCTIONS_TO_OVERWRITE])) >> 2;
    tramp_ptr[NUM_INSTRUCTIONS_TO_OVERWRITE] = 0x14000000 | (tramp_offset & 0x3FFFFFF);

    const auto hook_offset = (reinterpret_cast<uintptr_t>(replacement) - actual_target_address) >> 2;
    hook_ptr[0] = 0x14000000 | (hook_offset & 0x3FFFFFF);

    trampolines[target_address] = trampoline;
#else
    auto trampoline_ptr = reinterpret_cast<uint32_t*>(trampoline);
    auto target_ptr = reinterpret_cast<uint32_t*>(actual_target_address);

    if (is_thumb_mode) {
        const auto jump_back_address = (actual_target_address + overwrite_size) | 1;
        const auto replacement_address = reinterpret_cast<uintptr_t>(replacement) | 1;

        trampoline_ptr[overwrite_size / INSTRUCTION_SIZE_ARM] = 0xF8DFD000;
        trampoline_ptr[overwrite_size / INSTRUCTION_SIZE_ARM + 1] = jump_back_address;

        target_ptr[0] = 0xF8DFD000;
        target_ptr[1] = replacement_address;

        for (int i = 2; i < overwrite_size / INSTRUCTION_SIZE_ARM; ++i) {
            target_ptr[i] = 0xBF00BF00;
        }
    } else {
        const auto jump_back_address = actual_target_address + overwrite_size;

        trampoline_ptr[overwrite_size / INSTRUCTION_SIZE_ARM] = 0xE51FF004;
        trampoline_ptr[overwrite_size / INSTRUCTION_SIZE_ARM + 1] = jump_back_address;
        
        target_ptr[0] = 0xE51FF004;
        target_ptr[1] = reinterpret_cast<uint32_t>(replacement);

        for (int i = 2; i < overwrite_size / INSTRUCTION_SIZE_ARM; ++i) {
            target_ptr[i] = 0xE1A00000;
        }
    }
    
    trampolines[target_address] = trampoline;
#endif

    __builtin_clear_cache(reinterpret_cast<char*>(actual_target_address),
                          reinterpret_cast<char*>(actual_target_address + overwrite_size));
    
    return true;
}

template<typename T>
T hook_mgr::get_original(uintptr_t target_address) {
    const auto it = trampolines.find(target_address);
    if (it == trampolines.end()) {
        return nullptr;
    }
#ifdef __aarch64__
    return reinterpret_cast<T>(it->second);
#else
    const auto is_thumb_mode = IS_THUMB_MODE(target_address);
    return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(it->second) | (is_thumb_mode ? 1 : 0));
#endif
}
