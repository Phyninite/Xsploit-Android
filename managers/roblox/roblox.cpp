#include "../memory/memory.hpp"
#include "../Update.hpp"
#include "../utilities/utility.hpp"

class rbx_mgr {
public:
    static void initialize() {
        utility_mgr::log("rbx_mgr initialized");
    }
    
    static void print(const char* message) {
        const auto rbx_print = reinterpret_cast<void(*)(int, const char*)>(
            mem_mgr::rebase(Addresses::rbx_print)
        );
        rbx_print(1, message);
    }
};
