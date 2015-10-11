#pragma once

#include "vmtypes.hpp"

// Map a memory region of nwords vmwords and return a pointer to it
// Will return nullptr if the allocation fails
vmword *plat_map_memory(size_t nwords);

// Unmap a memory region mapped with plat_map_memory
void plat_unmap_memory(vmword *mem, size_t nwords);
