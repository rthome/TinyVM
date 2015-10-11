#include "platform.hpp"

#include <sys/mman.h>

vmword *plat_map_memory(size_t nwords)
{
    void *mem_ptr = mmap(nullptr, nwords * sizeof(vmword), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem_ptr == MAP_FAILED)
        return nullptr;
    return static_cast<vmword*>(mem_ptr);
}

void plat_unmap_memory(vmword *mem, size_t nwords)
{
    int res = munmap(mem, nwords * sizeof(vmword));
}
