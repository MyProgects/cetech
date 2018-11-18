//==============================================================================
// includes
//==============================================================================
#include <cstdio>

extern "C" {
#include <celib/allocator.h>
#include <celib/array.inl>

#include <celib/api_system.h>
#include <celib/config.h>
#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/hashlib.h>
#include <celib/os.h>
#include <celib/private/api_private.h>
#include <celib/cdb.h>
#include <celib/ebus.h>
#include <celib/log.h>

#include <cetech/kernel/kernel.h>
#include <cetech/resource/resource.h>
#include <cetech/renderer/renderer.h>
#include <cetech/machine/machine.h>
#include <cetech/ecs/ecs.h>
}

#include "bgfx/c99/bgfx.h"
#include "bgfx/c99/platform.h"

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct _G {
    ct_renderender_on_render *on_render;
    ce_window *main_window;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;

    bool capture;
    bool vsync;
    bool need_reset;
    uint64_t config;
    ce_alloc *allocator;
} _G = {};


static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}




//==============================================================================
// Interface
//==============================================================================

static void renderer_create() {

    if (!ce_cdb_a0->read_uint64(_G.config, CONFIG_DAEMON, 0)) {
        uint32_t w, h;
        w = ce_cdb_a0->read_uint64(_G.config, CONFIG_SCREEN_X, 0);
        h = ce_cdb_a0->read_uint64(_G.config, CONFIG_SCREEN_Y, 0);
        _G.size_width = w;
        _G.size_height = h;

        intptr_t wid = ce_cdb_a0->read_uint64(_G.config, CONFIG_WID, 0);

        char title[128] = {};
        snprintf(title, CE_ARRAY_LEN(title), "cetech");

        if (wid == 0) {
            bool fullscreen = ce_cdb_a0->read_uint64(_G.config,
                                                     CONFIG_SCREEN_FULLSCREEN,
                                                     0) > 0;

            uint32_t flags = WINDOW_NOFLAG;

            flags |= fullscreen ? WINDOW_FULLSCREEN : WINDOW_NOFLAG;
            flags |= WINDOW_RESIZABLE;

            _G.main_window = ce_os_a0->window->create(
                    _G.allocator,
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    w, h,
                    flags);

        } else {
            _G.main_window = ce_os_a0->window->create_from(_G.allocator,
                                                           (void *) wid);
        }
    }

    bgfx_platform_data_t pd = {NULL};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx_set_platform_data(&pd);

    // TODO: from config

    bgfx_init_t init;
    bgfx_init_ctor(&init);

    const char *rtype = ce_cdb_a0->read_str(ce_config_a0->obj(),
                                            CONFIG_RENDERER_TYPE, "");


    bool invalid = true;

    static struct {
        const char *k;
        bgfx_renderer_type_t v;
    } _str_to_render_type[] = {
            {.k = "noop", .v = BGFX_RENDERER_TYPE_NOOP},
            {.k = "opengl", .v = BGFX_RENDERER_TYPE_OPENGL},
            {.k = "metal", .v = BGFX_RENDERER_TYPE_METAL},

            {.k = "",
                    .v =
#if CE_PLATFORM_LINUX
                    BGFX_RENDERER_TYPE_OPENGL
#elif CE_PLATFORM_OSX
                    BGFX_RENDERER_TYPE_METAL// metal in future
#endif
            },
    };

    for (int i = 0; i < CE_ARRAY_LEN(_str_to_render_type); ++i) {
        if (strcmp(rtype, _str_to_render_type[i].k) == 0) {
            init.type = _str_to_render_type[i].v;
            invalid = false;
            break;
        }
    }

    if (invalid) {
        ce_log_a0->error("renderer", "Invalid render type '%s', force to noop",
                         rtype);
        init.type = BGFX_RENDERER_TYPE_NOOP;
    }


    bgfx_init(&init);

    _G.main_window->size(_G.main_window->inst, &_G.size_width, &_G.size_height);
    bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(), BGFX_TEXTURE_FORMAT_COUNT);
    //_G.main_window->update(_G.main_window);

    _G.need_reset = true;
}


static void renderer_set_debug(int debug) {
    if (debug) {
        bgfx_set_debug(BGFX_DEBUG_STATS);
    } else {
        bgfx_set_debug(BGFX_DEBUG_NONE);
    }
}


static void renderer_get_size(uint32_t *width,
                              uint32_t *height) {
    *width = _G.size_width;
    *height = _G.size_height;
}


static void on_resize(uint64_t type, void* event) {
    _G.need_reset = 1;

    struct ebus_cdb_event* ev = (struct ebus_cdb_event*)event;

    _G.size_width = ce_cdb_a0->read_uint64(ev->obj, CT_MACHINE_WINDOW_WIDTH, 0);
    _G.size_height = ce_cdb_a0->read_uint64(ev->obj, CT_MACHINE_WINDOW_HEIGHT, 0);
}

static void on_begin_render(uint64_t type, void* _event) {
    if (_G.need_reset) {
        _G.need_reset = 0;

        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(), BGFX_TEXTURE_FORMAT_COUNT);
    }
}

static void on_render(uint64_t type, void* _event) {
    bgfx_frame(false);
}

static struct ct_renderer_a0 rendderer_api = {
        .create = renderer_create,
        .set_debug = renderer_set_debug,
        .get_size = renderer_get_size,


///
        .vertex_decl_begin = reinterpret_cast<void (*)(ct_render_vertex_decl_t *,
                                                       ct_render_renderer_type_t)>(bgfx_vertex_decl_begin),
        .vertex_decl_add = reinterpret_cast<void (*)(ct_render_vertex_decl_t *,
                                                     ct_render_attrib_t,
                                                     uint8_t,
                                                     ct_render_attrib_type_t,
                                                     bool,
                                                     bool)>(bgfx_vertex_decl_add),
        .vertex_decl_skip = reinterpret_cast<void (*)(ct_render_vertex_decl_t *,
                                                      uint8_t)>(bgfx_vertex_decl_skip),
        .vertex_decl_end = reinterpret_cast<void (*)(ct_render_vertex_decl_t *)>(bgfx_vertex_decl_end),
        .vertex_pack = reinterpret_cast<void (*)(const float *,
                                                 bool,
                                                 ct_render_attrib_t,
                                                 const ct_render_vertex_decl_t *,
                                                 void *,
                                                 uint32_t)>(bgfx_vertex_pack),
        .vertex_unpack = reinterpret_cast<void (*)(float *,
                                                   ct_render_attrib_t,
                                                   const ct_render_vertex_decl_t *,
                                                   const void *,
                                                   uint32_t)>(bgfx_vertex_unpack),
        .vertex_convert = reinterpret_cast<void (*)(const ct_render_vertex_decl_t *,
                                                    void *,
                                                    const ct_render_vertex_decl_t *,
                                                    const void *,
                                                    uint32_t)>(bgfx_vertex_convert),
        .weld_vertices = reinterpret_cast<uint16_t (*)(uint16_t *,
                                                       const ct_render_vertex_decl_t *,
                                                       const void *,
                                                       uint16_t,
                                                       float)>(bgfx_weld_vertices),
        .topology_convert = reinterpret_cast<uint32_t (*)(ct_render_topology_convert_t,
                                                          void *,
                                                          uint32_t,
                                                          const void *,
                                                          uint32_t,
                                                          bool)>(bgfx_topology_convert),
        .topology_sort_tri_list = reinterpret_cast<void (*)(ct_render_topology_sort_t,
                                                            void *,
                                                            uint32_t,
                                                            const float *,
                                                            const float *,
                                                            const void *,
                                                            uint32_t,
                                                            const void *,
                                                            uint32_t,
                                                            bool)>(bgfx_topology_sort_tri_list),
        .get_supported_renderers = reinterpret_cast<uint8_t (*)(uint8_t,
                                                                ct_render_renderer_type_t *)>(bgfx_get_supported_renderers),
        .get_renderer_name = reinterpret_cast<const char *(*)(ct_render_renderer_type_t)>(bgfx_get_renderer_name),
        .frame = bgfx_frame,
        .get_renderer_type = reinterpret_cast<ct_render_renderer_type_t (*)()>(bgfx_get_renderer_type),
        .get_caps = reinterpret_cast<const ct_render_caps_t *(*)()>(bgfx_get_caps),
        .get_stats = reinterpret_cast<const ct_render_stats_t *(*)()>(bgfx_get_stats),
        .alloc = reinterpret_cast<const ct_render_memory_t *(*)(uint32_t)>(bgfx_alloc),
        .copy = reinterpret_cast<const ct_render_memory_t *(*)(const void *,
                                                               uint32_t)>(bgfx_copy),
        .make_ref = reinterpret_cast<const ct_render_memory_t *(*)(const void *,
                                                                   uint32_t)>(bgfx_make_ref),
        .make_ref_release = reinterpret_cast<const ct_render_memory_t *(*)(const void *,
                                                                           uint32_t,
                                                                           ct_render_release_fn_t,
                                                                           void *)>(bgfx_make_ref_release),
        .dbg_text_clear = bgfx_dbg_text_clear,
        .dbg_text_printf = bgfx_dbg_text_printf,
        .dbg_text_vprintf = bgfx_dbg_text_vprintf,
        .dbg_text_image = bgfx_dbg_text_image,
        .create_index_buffer = reinterpret_cast<ct_render_index_buffer_handle_t (*)(const ct_render_memory_t *,
                                                                                    uint16_t)>(bgfx_create_index_buffer),
        .destroy_index_buffer = reinterpret_cast<void (*)(ct_render_index_buffer_handle_t)>(bgfx_destroy_index_buffer),
        .create_vertex_buffer = reinterpret_cast<ct_render_vertex_buffer_handle_t (*)(const ct_render_memory_t *,
                                                                                      const ct_render_vertex_decl_t *,
                                                                                      uint16_t)>(bgfx_create_vertex_buffer),
        .destroy_vertex_buffer = reinterpret_cast<void (*)(ct_render_vertex_buffer_handle_t)>(bgfx_destroy_vertex_buffer),
        .create_dynamic_index_buffer = reinterpret_cast<ct_render_dynamic_index_buffer_handle_t (*)(uint32_t,
                                                                                                    uint16_t)>(bgfx_create_dynamic_index_buffer),
        .create_dynamic_index_buffer_mem = reinterpret_cast<ct_render_dynamic_index_buffer_handle_t (*)(const ct_render_memory_t *,
                                                                                                        uint16_t)>(bgfx_create_dynamic_index_buffer_mem),
        .update_dynamic_index_buffer = reinterpret_cast<void (*)(ct_render_dynamic_index_buffer_handle_t,
                                                                 uint32_t,
                                                                 const ct_render_memory_t *)>(bgfx_update_dynamic_index_buffer),
        .destroy_dynamic_index_buffer = reinterpret_cast<void (*)(ct_render_dynamic_index_buffer_handle_t)>(bgfx_destroy_dynamic_index_buffer),
        .create_dynamic_vertex_buffer = reinterpret_cast<ct_render_dynamic_vertex_buffer_handle_t (*)(uint32_t,
                                                                                                      const ct_render_vertex_decl_t *,
                                                                                                      uint16_t)>(bgfx_create_dynamic_vertex_buffer),
        .create_dynamic_vertex_buffer_mem = reinterpret_cast<ct_render_dynamic_vertex_buffer_handle_t (*)(const ct_render_memory_t *,
                                                                                                          const ct_render_vertex_decl_t *,
                                                                                                          uint16_t)>(bgfx_create_dynamic_vertex_buffer_mem),
        .update_dynamic_vertex_buffer = reinterpret_cast<void (*)(ct_render_dynamic_vertex_buffer_handle_t,
                                                                  uint32_t,
                                                                  const ct_render_memory_t *)>(bgfx_update_dynamic_vertex_buffer),
        .destroy_dynamic_vertex_buffer = reinterpret_cast<void (*)(ct_render_dynamic_vertex_buffer_handle_t)>(bgfx_destroy_dynamic_vertex_buffer),
        .get_avail_transient_index_buffer = bgfx_get_avail_transient_index_buffer,
        .get_avail_transient_vertex_buffer = reinterpret_cast<uint32_t (*)(uint32_t,
                                                                           const ct_render_vertex_decl_t *)>(bgfx_get_avail_transient_vertex_buffer),
        .get_avail_instance_data_buffer = bgfx_get_avail_instance_data_buffer,
        .alloc_transient_index_buffer = reinterpret_cast<void (*)(ct_render_transient_index_buffer_t *,
                                                                  uint32_t)>(bgfx_alloc_transient_index_buffer),
        .alloc_transient_vertex_buffer = reinterpret_cast<void (*)(ct_render_transient_vertex_buffer_t *,
                                                                   uint32_t,
                                                                   const ct_render_vertex_decl_t *)>(bgfx_alloc_transient_vertex_buffer),
        .alloc_transient_buffers = reinterpret_cast<bool (*)(ct_render_transient_vertex_buffer_t *,
                                                             const ct_render_vertex_decl_t *,
                                                             uint32_t,
                                                             ct_render_transient_index_buffer_t *,
                                                             uint32_t)>(bgfx_alloc_transient_buffers),
        .alloc_instance_data_buffer = reinterpret_cast<void (*)(ct_render_instance_data_buffer_t *,
                                                                uint32_t,
                                                                uint16_t)>(bgfx_alloc_instance_data_buffer),
        .create_indirect_buffer = reinterpret_cast<ct_render_indirect_buffer_handle_t (*)(uint32_t)>(bgfx_create_indirect_buffer),
        .destroy_indirect_buffer = reinterpret_cast<void (*)(ct_render_indirect_buffer_handle_t)>(bgfx_destroy_indirect_buffer),
        .create_shader = reinterpret_cast<ct_render_shader_handle_t (*)(const ct_render_memory_t *)>(bgfx_create_shader),
        .get_shader_uniforms = reinterpret_cast<uint16_t (*)(ct_render_shader_handle_t,
                                                             ct_render_uniform_handle_t *,
                                                             uint16_t)>(bgfx_get_shader_uniforms),
        .set_shader_name = reinterpret_cast<void (*)(ct_render_shader_handle_t,
                                                     const char *)>(bgfx_set_shader_name),
        .destroy_shader = reinterpret_cast<void (*)(ct_render_shader_handle_t)>(bgfx_destroy_shader),
        .create_program = reinterpret_cast<ct_render_program_handle_t (*)(ct_render_shader_handle_t,
                                                                          ct_render_shader_handle_t,
                                                                          bool)>(bgfx_create_program),
        .create_compute_program = reinterpret_cast<ct_render_program_handle_t (*)(ct_render_shader_handle_t,
                                                                                  bool)>(bgfx_create_compute_program),
        .destroy_program = reinterpret_cast<void (*)(ct_render_program_handle_t)>(bgfx_destroy_program),
        .is_texture_valid = reinterpret_cast<bool (*)(uint16_t,
                                                      bool,
                                                      uint16_t,
                                                      ct_render_texture_format_t,
                                                      uint32_t)>(bgfx_is_texture_valid),
        .calc_texture_size = reinterpret_cast<void (*)(ct_render_texture_info_t *,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       bool,
                                                       bool,
                                                       uint16_t,
                                                       ct_render_texture_format_t)>(bgfx_calc_texture_size),
        .create_texture = reinterpret_cast<ct_render_texture_handle_t (*)(const ct_render_memory_t *,
                                                                          uint64_t,
                                                                          uint8_t,
                                                                          ct_render_texture_info_t *)>(bgfx_create_texture),
        .create_texture_2d = reinterpret_cast<ct_render_texture_handle_t (*)(uint16_t,
                                                                             uint16_t,
                                                                             bool,
                                                                             uint16_t,
                                                                             ct_render_texture_format_t,
                                                                             uint64_t,
                                                                             const ct_render_memory_t *)>(bgfx_create_texture_2d),
        .create_texture_2d_scaled = reinterpret_cast<ct_render_texture_handle_t (*)(ct_render_backbuffer_ratio_t,
                                                                                    bool,
                                                                                    uint16_t,
                                                                                    ct_render_texture_format_t,
                                                                                    uint64_t)>(bgfx_create_texture_2d_scaled),
        .create_texture_3d = reinterpret_cast<ct_render_texture_handle_t (*)(uint16_t,
                                                                             uint16_t,
                                                                             uint16_t,
                                                                             bool,
                                                                             ct_render_texture_format_t,
                                                                             uint64_t,
                                                                             const ct_render_memory_t *)>(bgfx_create_texture_3d),
        .create_texture_cube = reinterpret_cast<ct_render_texture_handle_t (*)(uint16_t,
                                                                               bool,
                                                                               uint16_t,
                                                                               ct_render_texture_format_t,
                                                                               uint64_t,
                                                                               const ct_render_memory_t *)>(bgfx_create_texture_cube),
        .update_texture_2d = reinterpret_cast<void (*)(ct_render_texture_handle_t,
                                                       uint16_t,
                                                       uint8_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       const ct_render_memory_t *,
                                                       uint16_t)>(bgfx_update_texture_2d),
        .update_texture_3d = reinterpret_cast<void (*)(ct_render_texture_handle_t,
                                                       uint8_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       uint16_t,
                                                       const ct_render_memory_t *)>(bgfx_update_texture_3d),
        .update_texture_cube = reinterpret_cast<void (*)(ct_render_texture_handle_t,
                                                         uint16_t,
                                                         uint8_t,
                                                         uint8_t,
                                                         uint16_t,
                                                         uint16_t,
                                                         uint16_t,
                                                         uint16_t,
                                                         const ct_render_memory_t *,
                                                         uint16_t)>(bgfx_update_texture_cube),
        .read_texture = reinterpret_cast<uint32_t (*)(ct_render_texture_handle_t,
                                                      void *,
                                                      uint8_t)>(bgfx_read_texture),
        .set_texture_name = reinterpret_cast<void (*)(ct_render_texture_handle_t,
                                                      const char *)>(bgfx_set_texture_name),
        .destroy_texture = reinterpret_cast<void (*)(ct_render_texture_handle_t)>(bgfx_destroy_texture),
        .create_frame_buffer = reinterpret_cast<ct_render_frame_buffer_handle_t (*)(uint16_t,
                                                                                    uint16_t,
                                                                                    ct_render_texture_format_t,
                                                                                    uint64_t)>(bgfx_create_frame_buffer),
        .create_frame_buffer_scaled = reinterpret_cast<ct_render_frame_buffer_handle_t (*)(ct_render_backbuffer_ratio_t,
                                                                                           ct_render_texture_format_t,
                                                                                           uint64_t)>(bgfx_create_frame_buffer_scaled),
        .create_frame_buffer_from_attachment = reinterpret_cast<ct_render_frame_buffer_handle_t (*)(uint8_t,
                                                                                                    const ct_render_attachment_t *,
                                                                                                    bool)>(bgfx_create_frame_buffer_from_attachment),
        .create_frame_buffer_from_nwh = reinterpret_cast<ct_render_frame_buffer_handle_t (*)(void *,
                                                                                             uint16_t,
                                                                                             uint16_t,
                                                                                             ct_render_texture_format_t)>(bgfx_create_frame_buffer_from_nwh),
        .create_frame_buffer_from_handles = reinterpret_cast<ct_render_frame_buffer_handle_t (*)(uint8_t,
                                                                                                 const ct_render_texture_handle_t *,
                                                                                                 bool)>(bgfx_create_frame_buffer_from_handles),
        .get_texture = reinterpret_cast<ct_render_texture_handle_t (*)(ct_render_frame_buffer_handle_t,
                                                                       uint8_t)>(bgfx_get_texture),
        .destroy_frame_buffer = reinterpret_cast<void (*)(ct_render_frame_buffer_handle_t)>(bgfx_destroy_frame_buffer),
        .create_uniform = reinterpret_cast<ct_render_uniform_handle_t (*)(const char *,
                                                                          ct_render_uniform_type_t,
                                                                          uint16_t)>(bgfx_create_uniform),
        .get_uniform_info = reinterpret_cast<void (*)(ct_render_uniform_handle_t,
                                                      ct_render_uniform_info_t *)>(bgfx_get_uniform_info),
        .destroy_uniform = reinterpret_cast<void (*)(ct_render_uniform_handle_t)>(bgfx_destroy_uniform),
        .create_occlusion_query = reinterpret_cast<ct_render_occlusion_query_handle_t (*)()>(bgfx_create_occlusion_query),
        .get_result = reinterpret_cast<ct_render_occlusion_query_result_t (*)(ct_render_occlusion_query_handle_t,
                                                                              int32_t *)>(bgfx_get_result),
        .destroy_occlusion_query = reinterpret_cast<void (*)(ct_render_occlusion_query_handle_t)>(bgfx_destroy_occlusion_query),
        .set_palette_color = bgfx_set_palette_color,
        .set_view_name = bgfx_set_view_name,
        .set_view_rect = bgfx_set_view_rect,
        .set_view_scissor = bgfx_set_view_scissor,
        .set_view_clear = bgfx_set_view_clear,
        .set_view_clear_mrt = bgfx_set_view_clear_mrt,
        .set_view_mode = reinterpret_cast<void (*)(ct_render_view_id_t,
                                                   ct_render_view_mode_t)>(bgfx_set_view_mode),
        .set_view_frame_buffer = reinterpret_cast<void (*)(ct_render_view_id_t,
                                                           ct_render_frame_buffer_handle_t)>(bgfx_set_view_frame_buffer),
        .set_view_transform = bgfx_set_view_transform,
        .set_view_transform_stereo = bgfx_set_view_transform_stereo,
        .set_view_order = bgfx_set_view_order,


        .set_marker = reinterpret_cast<void (*)(
                const char *)>(bgfx_set_marker),
        .set_state = reinterpret_cast<void (*)(
                uint64_t,
                uint32_t)>(bgfx_set_state),
        .set_condition = reinterpret_cast<void (*)(
                ct_render_occlusion_query_handle_t,
                bool)>(bgfx_set_condition),
        .set_stencil = reinterpret_cast<void (*)(
                uint32_t,
                uint32_t)>(bgfx_set_stencil),
        .set_scissor = reinterpret_cast<uint16_t (*)(
                uint16_t,
                uint16_t,
                uint16_t,
                uint16_t)>(bgfx_set_scissor),
        .set_scissor_cached = reinterpret_cast<void (*)(
                uint16_t)>(bgfx_set_scissor_cached),
        .set_transform = reinterpret_cast<uint32_t (*)(
                const void *,
                uint16_t)>(bgfx_set_transform),
        .alloc_transform = reinterpret_cast<uint32_t (*)(
                ct_render_transform_t *,
                uint16_t)>(bgfx_alloc_transform),
        .set_transform_cached = reinterpret_cast<void (*)(
                uint32_t,
                uint16_t)>(bgfx_set_transform_cached),
        .set_uniform = reinterpret_cast<void (*)(
                ct_render_uniform_handle_t,
                const void *,
                uint16_t)>(bgfx_set_uniform),
        .set_index_buffer = reinterpret_cast<void (*)(
                ct_render_index_buffer_handle_t,
                uint32_t,
                uint32_t)>(bgfx_set_index_buffer),
        .set_dynamic_index_buffer = reinterpret_cast<void (*)(
                ct_render_dynamic_index_buffer_handle_t,
                uint32_t,
                uint32_t)>(bgfx_set_dynamic_index_buffer),
        .set_transient_index_buffer = reinterpret_cast<void (*)(
                const ct_render_transient_index_buffer_t *,
                uint32_t,
                uint32_t)>(bgfx_set_transient_index_buffer),
        .set_vertex_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_vertex_buffer_handle_t,
                uint32_t,
                uint32_t)>(bgfx_set_vertex_buffer),
        .set_dynamic_vertex_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_dynamic_vertex_buffer_handle_t,
                uint32_t,
                uint32_t)>(bgfx_set_dynamic_vertex_buffer),
        .set_transient_vertex_buffer = reinterpret_cast<void (*)(
                uint8_t,
                const ct_render_transient_vertex_buffer_t *,
                uint32_t,
                uint32_t)>(bgfx_set_transient_vertex_buffer),
        .set_instance_data_buffer = reinterpret_cast<void (*)(
                const ct_render_instance_data_buffer_t *,
                uint32_t,
                uint32_t)>(bgfx_set_instance_data_buffer),
        .set_instance_data_from_vertex_buffer = reinterpret_cast<void (*)(
                ct_render_vertex_buffer_handle_t,
                uint32_t,
                uint32_t)>(bgfx_set_instance_data_from_vertex_buffer),
        .set_instance_data_from_dynamic_vertex_buffer = reinterpret_cast<void (*)(
                ct_render_dynamic_vertex_buffer_handle_t,
                uint32_t,
                uint32_t)>(bgfx_set_instance_data_from_dynamic_vertex_buffer),
        .set_texture = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_uniform_handle_t,
                ct_render_texture_handle_t,
                uint32_t)>(bgfx_set_texture),
        .touch = reinterpret_cast<void (*)(
                ct_render_view_id_t)>(bgfx_touch),
        .submit = reinterpret_cast<void (*)(
                ct_render_view_id_t,
                ct_render_program_handle_t,
                int32_t,
                bool)>(bgfx_submit),
        .submit_occlusion_query = reinterpret_cast<void (*)(
                ct_render_view_id_t,
                ct_render_program_handle_t,
                ct_render_occlusion_query_handle_t,
                int32_t,
                bool)>(bgfx_submit_occlusion_query),
        .submit_indirect = reinterpret_cast<void (*)(
                ct_render_view_id_t,
                ct_render_program_handle_t,
                ct_render_indirect_buffer_handle_t,
                uint16_t,
                uint16_t,
                int32_t,
                bool)>(bgfx_submit_indirect),
        .set_image = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_texture_handle_t,
                uint8_t,
                ct_render_access_t,
                ct_render_texture_format_t)>(bgfx_set_image),
        .set_compute_index_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_index_buffer_handle_t,
                ct_render_access_t)>(bgfx_set_compute_index_buffer),
        .set_compute_vertex_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_vertex_buffer_handle_t,
                ct_render_access_t)>(bgfx_set_compute_vertex_buffer),
        .set_compute_dynamic_index_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_dynamic_index_buffer_handle_t,
                ct_render_access_t)>(bgfx_set_compute_dynamic_index_buffer),
        .set_compute_dynamic_vertex_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_dynamic_vertex_buffer_handle_t,
                ct_render_access_t)>(bgfx_set_compute_dynamic_vertex_buffer),
        .set_compute_indirect_buffer = reinterpret_cast<void (*)(
                uint8_t,
                ct_render_indirect_buffer_handle_t,
                ct_render_access_t)>(bgfx_set_compute_indirect_buffer),
        .dispatch = reinterpret_cast<void (*)(
                ct_render_view_id_t,
                ct_render_program_handle_t,
                uint32_t,
                uint32_t,
                uint32_t,
                uint8_t)>(bgfx_dispatch),
        .dispatch_indirect = reinterpret_cast<void (*)(
                ct_render_view_id_t,
                ct_render_program_handle_t,
                ct_render_indirect_buffer_handle_t,
                uint16_t,
                uint16_t,
                uint8_t)>(bgfx_dispatch_indirect),
        .discard = reinterpret_cast<void (*)()>(bgfx_discard),
        .blit = reinterpret_cast<void (*)(
                ct_render_view_id_t,
                ct_render_texture_handle_t,
                uint8_t,
                uint16_t,
                uint16_t,
                uint16_t,
                ct_render_texture_handle_t,
                uint8_t,
                uint16_t,
                uint16_t,
                uint16_t,
                uint16_t,
                uint16_t,
                uint16_t)>(bgfx_blit),


        .encoder_set_marker = reinterpret_cast<void (*)(ct_render_encoder *,
                                                        const char *)>(bgfx_encoder_set_marker),
        .encoder_set_state = reinterpret_cast<void (*)(ct_render_encoder *,
                                                       uint64_t,
                                                       uint32_t)>(bgfx_encoder_set_state),
        .encoder_set_condition = reinterpret_cast<void (*)(ct_render_encoder *,
                                                           ct_render_occlusion_query_handle_t,
                                                           bool)>(bgfx_encoder_set_condition),
        .encoder_set_stencil = reinterpret_cast<void (*)(ct_render_encoder *,
                                                         uint32_t,
                                                         uint32_t)>(bgfx_encoder_set_stencil),
        .encoder_set_scissor = reinterpret_cast<uint16_t (*)(ct_render_encoder *,
                                                             uint16_t,
                                                             uint16_t,
                                                             uint16_t,
                                                             uint16_t)>(bgfx_encoder_set_scissor),
        .encoder_set_scissor_cached = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                uint16_t)>(bgfx_encoder_set_scissor_cached),
        .encoder_set_transform = reinterpret_cast<uint32_t (*)(ct_render_encoder *,
                                                               const void *,
                                                               uint16_t)>(bgfx_encoder_set_transform),
        .encoder_alloc_transform = reinterpret_cast<uint32_t (*)(ct_render_encoder *,
                                                                 ct_render_transform_t *,
                                                                 uint16_t)>(bgfx_encoder_alloc_transform),
        .encoder_set_transform_cached = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                  uint32_t,
                                                                  uint16_t)>(bgfx_encoder_set_transform_cached),
        .encoder_set_uniform = reinterpret_cast<void (*)(ct_render_encoder *,
                                                         ct_render_uniform_handle_t,
                                                         const void *,
                                                         uint16_t)>(bgfx_encoder_set_uniform),
        .encoder_set_index_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                              ct_render_index_buffer_handle_t,
                                                              uint32_t,
                                                              uint32_t)>(bgfx_encoder_set_index_buffer),
        .encoder_set_dynamic_index_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                      ct_render_dynamic_index_buffer_handle_t,
                                                                      uint32_t,
                                                                      uint32_t)>(bgfx_encoder_set_dynamic_index_buffer),
        .encoder_set_transient_index_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                        const ct_render_transient_index_buffer_t *,
                                                                        uint32_t,
                                                                        uint32_t)>(bgfx_encoder_set_transient_index_buffer),
        .encoder_set_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                               uint8_t,
                                                               ct_render_vertex_buffer_handle_t,
                                                               uint32_t,
                                                               uint32_t)>(bgfx_encoder_set_vertex_buffer),
        .encoder_set_dynamic_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                       uint8_t,
                                                                       ct_render_dynamic_vertex_buffer_handle_t,
                                                                       uint32_t,
                                                                       uint32_t)>(bgfx_encoder_set_dynamic_vertex_buffer),
        .encoder_set_transient_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                         uint8_t,
                                                                         const ct_render_transient_vertex_buffer_t *,
                                                                         uint32_t,
                                                                         uint32_t)>(bgfx_encoder_set_transient_vertex_buffer),
        .encoder_set_instance_data_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                      const ct_render_instance_data_buffer_t *,
                                                                      uint32_t,
                                                                      uint32_t)>(bgfx_encoder_set_instance_data_buffer),
        .encoder_set_instance_data_from_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                                  ct_render_vertex_buffer_handle_t,
                                                                                  uint32_t,
                                                                                  uint32_t)>(bgfx_encoder_set_instance_data_from_vertex_buffer),
        .encoder_set_instance_data_from_dynamic_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                                          ct_render_dynamic_vertex_buffer_handle_t,
                                                                                          uint32_t,
                                                                                          uint32_t)>(bgfx_encoder_set_instance_data_from_dynamic_vertex_buffer),
        .encoder_set_texture = reinterpret_cast<void (*)(ct_render_encoder *,
                                                         uint8_t,
                                                         ct_render_uniform_handle_t,
                                                         ct_render_texture_handle_t,
                                                         uint32_t)>(bgfx_encoder_set_texture),
        .encoder_touch = reinterpret_cast<void (*)(ct_render_encoder *,
                                                   ct_render_view_id_t)>(bgfx_encoder_touch),
        .encoder_submit = reinterpret_cast<void (*)(ct_render_encoder *,
                                                    ct_render_view_id_t,
                                                    ct_render_program_handle_t,
                                                    int32_t,
                                                    bool)>(bgfx_encoder_submit),
        .encoder_submit_occlusion_query = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                    ct_render_view_id_t,
                                                                    ct_render_program_handle_t,
                                                                    ct_render_occlusion_query_handle_t,
                                                                    int32_t,
                                                                    bool)>(bgfx_encoder_submit_occlusion_query),
        .encoder_submit_indirect = reinterpret_cast<void (*)(ct_render_encoder *,
                                                             ct_render_view_id_t,
                                                             ct_render_program_handle_t,
                                                             ct_render_indirect_buffer_handle_t,
                                                             uint16_t,
                                                             uint16_t,
                                                             int32_t,
                                                             bool)>(bgfx_encoder_submit_indirect),
        .encoder_set_image = reinterpret_cast<void (*)(ct_render_encoder *,
                                                       uint8_t,
                                                       ct_render_texture_handle_t,
                                                       uint8_t,
                                                       ct_render_access_t,
                                                       ct_render_texture_format_t)>(bgfx_encoder_set_image),
        .encoder_set_compute_index_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                      uint8_t,
                                                                      ct_render_index_buffer_handle_t,
                                                                      ct_render_access_t)>(bgfx_encoder_set_compute_index_buffer),
        .encoder_set_compute_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                       uint8_t,
                                                                       ct_render_vertex_buffer_handle_t,
                                                                       ct_render_access_t)>(bgfx_encoder_set_compute_vertex_buffer),
        .encoder_set_compute_dynamic_index_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                              uint8_t,
                                                                              ct_render_dynamic_index_buffer_handle_t,
                                                                              ct_render_access_t)>(bgfx_encoder_set_compute_dynamic_index_buffer),
        .encoder_set_compute_dynamic_vertex_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                               uint8_t,
                                                                               ct_render_dynamic_vertex_buffer_handle_t,
                                                                               ct_render_access_t)>(bgfx_encoder_set_compute_dynamic_vertex_buffer),
        .encoder_set_compute_indirect_buffer = reinterpret_cast<void (*)(ct_render_encoder *,
                                                                         uint8_t,
                                                                         ct_render_indirect_buffer_handle_t,
                                                                         ct_render_access_t)>(bgfx_encoder_set_compute_indirect_buffer),
        .encoder_dispatch = reinterpret_cast<void (*)(ct_render_encoder *,
                                                      ct_render_view_id_t,
                                                      ct_render_program_handle_t,
                                                      uint32_t,
                                                      uint32_t,
                                                      uint32_t,
                                                      uint8_t)>(bgfx_encoder_dispatch),
        .encoder_dispatch_indirect = reinterpret_cast<void (*)(ct_render_encoder *,
                                                               ct_render_view_id_t,
                                                               ct_render_program_handle_t,
                                                               ct_render_indirect_buffer_handle_t,
                                                               uint16_t,
                                                               uint16_t,
                                                               uint8_t)>(bgfx_encoder_dispatch_indirect),
        .encoder_discard = reinterpret_cast<void (*)(ct_render_encoder *)>(bgfx_encoder_discard),
        .encoder_blit = reinterpret_cast<void (*)(ct_render_encoder *,
                                                  ct_render_view_id_t,
                                                  ct_render_texture_handle_t,
                                                  uint8_t,
                                                  uint16_t,
                                                  uint16_t,
                                                  uint16_t,
                                                  ct_render_texture_handle_t,
                                                  uint8_t,
                                                  uint16_t,
                                                  uint16_t,
                                                  uint16_t,
                                                  uint16_t,
                                                  uint16_t,
                                                  uint16_t)>(bgfx_encoder_blit),

        .request_screen_shot = reinterpret_cast<void (*)(ct_render_frame_buffer_handle_t,
                                                         const char *)>(bgfx_request_screen_shot),

};

struct ct_renderer_a0 *ct_renderer_a0 = &rendderer_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_renderer_a0", &rendderer_api);
}

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    ce_api_a0 = api;

    _G = {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj(),
    };

    ce_ebus_a0->connect(WINDOW_EBUS, EVENT_WINDOW_RESIZED, on_resize, 0);

    ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT,
                        on_begin_render, KERNEL_ORDER + 1);

    ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_UPDATE_EVENT,
                    on_render, RENDER_ORDER + 1);

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config);

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SCREEN_X)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_X, 1024);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SCREEN_Y)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_Y, 768);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SCREEN_FULLSCREEN)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_FULLSCREEN, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_DAEMON)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_DAEMON, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_WID)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_WID, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_RENDERER_TYPE)) {
        ce_cdb_a0->set_str(writer, CONFIG_RENDERER_TYPE, "");
    }

    ce_cdb_a0->write_commit(writer);


    _G.vsync = ce_cdb_a0->read_uint64(_G.config, CONFIG_SCREEN_VSYNC, 1) > 0;

    CE_INIT_API(api, ce_os_a0);

    renderer_create();

}

static void _shutdown() {
    if (!ce_cdb_a0->read_uint64(_G.config, CONFIG_DAEMON, 0)) {

        ce_array_free(_G.on_render, _G.allocator);

        bgfx_shutdown();
    }

    ce_ebus_a0->disconnect(WINDOW_EBUS, EVENT_WINDOW_RESIZED, on_resize);

    _G = {};
}

CE_MODULE_DEF(
        renderer,
        {
            CE_INIT_API(api, ce_config_a0);
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ct_machine_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_ebus_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            _shutdown();
        }
)