#include <string.h>
#include "graphics/lna_model.h"
#include "core/lna_memory_pool.h"
#include "core/lna_assert.h"
#include "core/lna_string.h"
#include "core/lna_file.h"

#define LNA_MODEL_FACE_POINT_COUNT 3
typedef struct lna_model_face_s
{
    uint32_t    position_indices[LNA_MODEL_FACE_POINT_COUNT];
    uint32_t    uv_indices[LNA_MODEL_FACE_POINT_COUNT];
    uint32_t    normal_indices[LNA_MODEL_FACE_POINT_COUNT];
} lna_model_face_t;

void lna_model_init_dev_mode(lna_model_t* model, const lna_model_config_t* config)
{
    lna_assert(model)
    lna_assert(config)
    lna_assert(config->filename)
    lna_assert(config->temp_lifetime_mem_pool)
    lna_assert(config->object_lifetime_mem_pool)

    lna_log_message("--------------------------");
    lna_log_message("load 3d object from file %s:", config->filename);
    lna_log_message("--------------------------");

    lna_file_content_t obj_file = { 0 };
    lna_file_debug_load(
        &obj_file,
        config->temp_lifetime_mem_pool,
        config->filename,
        false
        );

    uint32_t position_count = 0;
    uint32_t uv_count       = 0;
    uint32_t normal_count   = 0;
    uint32_t face_count     = 0;

    char* buffer_ptr = obj_file.content;
    while (buffer_ptr && *buffer_ptr != '\0')
    {
        if (lna_string_begins_with(buffer_ptr, "v "))
        {
            ++position_count;
        }
        else if (lna_string_begins_with(buffer_ptr, "vt "))
        {
            ++uv_count;
        }
        else if (lna_string_begins_with(buffer_ptr, "vn "))
        {
            ++normal_count;
        }
        else if (lna_string_begins_with(buffer_ptr, "f "))
        {
            ++face_count;
        }
        buffer_ptr = lna_string_go_to_next_line(buffer_ptr);
    }
    uint32_t vertex_count   = LNA_MODEL_FACE_POINT_COUNT * face_count;
    uint32_t index_count    = LNA_MODEL_FACE_POINT_COUNT * face_count;

    lna_log_message("3d object position count: %d", position_count);
    lna_log_message("3d object uv count      : %d", uv_count);
    lna_log_message("3d object normal count  : %d", normal_count);
    lna_log_message("3d object face count    : %d", face_count);
    lna_log_message("3d object vertex count  : %d", vertex_count);
    lna_log_message("3d object index count   : %d", index_count);

    lna_vec3_t*         positions   = position_count > 0    ? lna_memory_pool_reserve(config->temp_lifetime_mem_pool, sizeof(lna_vec3_t) * position_count)    : 0;
    lna_vec2_t*         uvs         = uv_count > 0          ? lna_memory_pool_reserve(config->temp_lifetime_mem_pool, sizeof(lna_vec2_t) * uv_count)          : 0;
    lna_vec3_t*         normals     = normal_count > 0      ? lna_memory_pool_reserve(config->temp_lifetime_mem_pool, sizeof(lna_vec3_t) * normal_count)      : 0;
    lna_model_face_t*   faces       = face_count > 0        ? lna_memory_pool_reserve(config->temp_lifetime_mem_pool, sizeof(lna_model_face_t) * face_count)  : 0;
    uint32_t*           indices     = face_count > 0        ? lna_memory_pool_reserve(config->temp_lifetime_mem_pool, sizeof(uint32_t) * index_count)         : 0;

    uint32_t position_index = 0;
    uint32_t uv_index       = 0;
    uint32_t normal_index   = 0;
    uint32_t face_index     = 0;
    buffer_ptr              = obj_file.content;
    while (buffer_ptr && *buffer_ptr != '\0')
    {
        if (lna_string_begins_with(buffer_ptr, "v "))
        {
            lna_assert(position_index < position_count)

            // FORMAT: v 1.000000 -1.000000 -1.000000
            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            positions[position_index].x = lna_string_parse_float(buffer_ptr);

            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            positions[position_index].y = lna_string_parse_float(buffer_ptr);

            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            positions[position_index].z = lna_string_parse_float(buffer_ptr);

            ++position_index;
        }
        else if (lna_string_begins_with(buffer_ptr, "vt "))
        {
            lna_assert(uv_index < uv_count)

            // FORMAT: vt 0.748573 0.750412
            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            uvs[uv_index].x = lna_string_parse_float(buffer_ptr);

            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            uvs[uv_index].y = lna_string_parse_float(buffer_ptr);

            ++uv_index;
        }
        else if (lna_string_begins_with(buffer_ptr, "vn "))
        {
            lna_assert(normal_index < normal_count)

            // FORMAT: v 1.000000 -1.000000 -1.000000
            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            normals[normal_index].x = lna_string_parse_float(buffer_ptr);

            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            normals[normal_index].y = lna_string_parse_float(buffer_ptr);

            buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
            buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
            normals[normal_index].z = lna_string_parse_float(buffer_ptr);

            ++normal_index;
        }
        else if (lna_string_begins_with(buffer_ptr, "f "))
        {
            lna_assert(face_index < face_count)

            // FORMAT: f 5/1/1 1/2/1 4/3/1
            for (uint32_t point_index = 0; point_index < LNA_MODEL_FACE_POINT_COUNT; ++point_index)
            {
                buffer_ptr = lna_string_go_to_next_space_character(buffer_ptr);
                buffer_ptr = lna_string_go_to_next_non_space_character(buffer_ptr);
                faces[face_index].position_indices[point_index] = (uint32_t)lna_string_parse_int(buffer_ptr);

                buffer_ptr = lna_string_go_to_next_character(buffer_ptr, '/');
                ++buffer_ptr;
                faces[face_index].uv_indices[point_index] = (uint32_t)lna_string_parse_int(buffer_ptr);

                buffer_ptr = lna_string_go_to_next_character(buffer_ptr, '/');
                ++buffer_ptr;
                faces[face_index].normal_indices[point_index] = (uint32_t)lna_string_parse_int(buffer_ptr);
            }
            ++face_index;
        }
        buffer_ptr = lna_string_go_to_next_line(buffer_ptr);
    }

    lna_assert(vertex_count > 0)
    model->vertices.count   = vertex_count;
    model->vertices.data    = lna_memory_pool_reserve(config->object_lifetime_mem_pool, vertex_count * sizeof(lna_model_vertex_t));

    uint32_t cur_index_count = 0;
    uint32_t cur_vertex_count = 0;
    for (face_index = 0; face_index < face_count; ++face_index)
    {
        for (uint32_t point_index = 0; point_index < LNA_MODEL_FACE_POINT_COUNT; ++point_index)
        {
            lna_model_vertex_t vertex;
            vertex.position.x   = positions[faces[face_index].position_indices[point_index] - 1].x;
            vertex.position.y   = positions[faces[face_index].position_indices[point_index] - 1].y;
            vertex.position.z   = positions[faces[face_index].position_indices[point_index] - 1].z;
            vertex.uv.x         = uvs[faces[face_index].uv_indices[point_index] - 1].x;
            vertex.uv.y         = uvs[faces[face_index].uv_indices[point_index] - 1].y;
            vertex.normal.x     = normals[faces[face_index].normal_indices[point_index] - 1].x;
            vertex.normal.y     = normals[faces[face_index].normal_indices[point_index] - 1].y;
            vertex.normal.z     = normals[faces[face_index].normal_indices[point_index] - 1].z;
            vertex.color.r      = 1.0f;
            vertex.color.g      = 1.0f;
            vertex.color.b      = 1.0f;
            vertex.color.a      = 1.0f;

            model->vertices.data[cur_vertex_count]  = vertex;
            indices[cur_index_count++]              = cur_vertex_count++;
        }
    }
    lna_assert(cur_index_count == index_count)

    lna_assert(index_count > 0)
    model->indices.count    = index_count;
    model->indices.data     = lna_memory_pool_reserve(config->object_lifetime_mem_pool, index_count * sizeof(uint32_t));
    memcpy(
        model->indices.data,
        indices,
        index_count * sizeof(uint32_t)
        );
}
