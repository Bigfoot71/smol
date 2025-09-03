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

#include <smol.h>

#include "./internal/sl__render.h"

/* === Public API === */

sl_mesh_id sl_mesh_create(const sl_vertex_3d_t* vertices, uint16_t v_count, const uint16_t* indices, uint32_t i_count)
{
    sl__mesh_t mesh = { 0 };

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, v_count * sizeof(sl_vertex_3d_t), vertices, GL_DYNAMIC_DRAW);

    if (indices != NULL && i_count > 0) {
        glGenBuffers(1, &mesh.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_count * sizeof(uint16_t), indices, GL_DYNAMIC_DRAW);
    }

    return sl__registry_add(&sl__render.reg_meshes, &mesh);
}

void sl_mesh_destroy(sl_mesh_id mesh)
{
    if (mesh == 0) {
        return;
    }

    sl__mesh_t* data = sl__registry_get(&sl__render.reg_meshes, mesh);
    if (data == NULL) return;

    glDeleteBuffers(1, &data->vbo);
    if (data->ebo > 0) {
        glDeleteBuffers(1, &data->ebo);
    }

    sl__registry_remove(&sl__render.reg_meshes, mesh);
}

void sl_mesh_update_vertices(sl_mesh_id mesh, const sl_vertex_3d_t* vertices, uint16_t count)
{
    if (vertices == NULL || count == 0) {
        return;
    }

    sl__mesh_t* data = sl__registry_get(&sl__render.reg_meshes, mesh);
    if (data == NULL) return;

    glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, 
        count * sizeof(sl_vertex_3d_t), 
        vertices
    );
}

void sl_mesh_update_indices(sl_mesh_id mesh, const uint16_t* indices, uint32_t count)
{
    if (indices == NULL || count == 0) {
        return;
    }

    sl__mesh_t* data = sl__registry_get(&sl__render.reg_meshes, mesh);
    if (data == NULL) return;

    if (data->ebo == 0) {
        glGenBuffers(1, &data->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint16_t), indices, GL_DYNAMIC_DRAW);
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER, 0, 
            count * sizeof(uint16_t), 
            indices
        );
    }
}
