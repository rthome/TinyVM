#include "platform.hpp"

#include <cstdlib>

vmword *plat_map_memory(size_t nwords)
{
    void *mem_pointer = calloc(nwords, sizeof(vmword));
    return static_cast<vmword*>(mem_pointer);
}

void plat_unmap_memory(vmword *mem)
{
    free(mem);
}
