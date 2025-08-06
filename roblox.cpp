#include "memory.hpp"
#include "Update.hpp"

class rbx_mgr {
public:
   static void initialize() {
       mem_mgr::get_lib_base("libroblox.so");
   }
   
   static void print(const char* message) {
       const auto rbx_print = reinterpret_cast<void(*)(int, const char*)>(
           mem_mgr::REBASE(Addresses::rbx_print)
       );
       rbx_print(1, message);
   }
};

__attribute__((constructor))
void on_load() {
   rbx_mgr::initialize();
   rbx_mgr::print("hi");
}
