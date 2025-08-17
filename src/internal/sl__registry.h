/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef SL__REGISTRY_H
#define SL__REGISTRY_H

#include "./sl__array.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    sl__array_t elements;       // Stores elements contiguously
    sl__array_t valid_flags;    // Stores whether an ID is valid (boolean array)
    sl__array_t free_ids;       // List of released IDs
    uint32_t next_id;           // Next ID to be assigned
    size_t elem_size;           // Size of an element
} sl__registry_t;

static inline sl__registry_t
sl__registry_create(size_t capacity, size_t elem_size)
{
    sl__registry_t registry = { 0 };
    registry.elements = sl__array_create(capacity, elem_size);
    registry.valid_flags = sl__array_create(capacity, sizeof(bool));
    registry.free_ids = sl__array_create(capacity, sizeof(uint32_t));
    registry.next_id = 1;
    registry.elem_size = elem_size;
    return registry;
}

static inline void
sl__registry_destroy(sl__registry_t* registry)
{
    sl__array_destroy(&registry->valid_flags);
    sl__array_destroy(&registry->free_ids);
    sl__array_destroy(&registry->elements);
}

static inline bool
sl__registry_is_valid(sl__registry_t* registry, uint32_t id)
{
    if (id == 0 || id >= registry->next_id) return false;
    return ((bool*)registry->valid_flags.data)[id - 1];
}

static inline uint32_t
sl__registry_add(sl__registry_t* registry, void* element)
{
    uint32_t id = 0;

    if (registry->free_ids.count > 0) {
        sl__array_pop_back(&registry->free_ids, &id);
    }
    else {
        sl__array_push_back(&registry->elements, NULL);
        sl__array_push_back(&registry->valid_flags, &(bool){true});
        id = registry->next_id++;
    }

    void* elem = sl__array_at(&registry->elements, id - 1);

    if (element) SDL_memcpy(elem, element, registry->elem_size);
    else SDL_memset(elem, 0, registry->elem_size);

    ((bool*)registry->valid_flags.data)[id - 1] = true;

    return id;
}

static inline void
sl__registry_remove(sl__registry_t* registry, uint32_t id)
{
    if (!sl__registry_is_valid(registry, id)) return;

    sl__array_push_back(&registry->free_ids, &id);
    ((bool*)registry->valid_flags.data)[id - 1] = false;
}

static inline void*
sl__registry_get(sl__registry_t* registry, uint32_t id)
{
    if (!sl__registry_is_valid(registry, id)) return NULL;
    return sl__array_at(&registry->elements, id - 1);
}

static inline uint32_t
sl__registry_get_allocated_count(sl__registry_t* registry)
{
    return (uint32_t)registry->elements.count;
}

#endif // SL__REGISTRY_H
