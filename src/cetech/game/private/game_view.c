#include <string.h>


#include <celib/macros.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/module.h"

#include "celib/api.h"


#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/renderer/renderer.h>
#include <cetech/editor/dock.h>
#include <cetech/renderer/gfx.h>
#include <cetech/game/game_system.h>

#include <cetech/debugui/debugui.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/ecs/ecs.h>

#define WINDOW_NAME "Game view"

static float step_dt = 16.0f;

static void on_menu(uint64_t content,
                    uint64_t context,
                    uint64_t selected_object) {
    static const char *label[] = {
            ICON_FA_PAUSE,
            ICON_FA_PLAY,
    };

    bool is_paused = ct_game_system_a0->is_paused();

    if (ct_debugui_a0->Button(label[is_paused], &CE_VEC2_ZERO)) {
        if (is_paused) {
            ct_game_system_a0->play();
        } else {
            ct_game_system_a0->pause();
        }
    }

    if (is_paused) {
        ct_debugui_a0->SameLine(0.0f, 4.0f);
        if (ct_debugui_a0->Button(ICON_FA_FORWARD, &CE_VEC2_ZERO)) {
            ct_game_system_a0->step(step_dt / 1000.0f);
        }

        ct_debugui_a0->SameLine(0.0f, 4.0f);
        ct_debugui_a0->InputFloat("delta", &step_dt, 0.0f, 0.0f, -1, 0);
    }

}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    ce_vec2_t size = ct_debugui_a0->GetContentRegionAvail();

    ct_world_t0 world = ct_game_system_a0->world();
    ct_entity_t0 viewport_ent = ct_ecs_q_a0->first(world, (ct_ecs_query_t0) {
            .all = CT_ECS_ARCHETYPE(VIEWPORT_COMPONENT),
    });

    viewport_component *viewport = ct_ecs_c_a0->get_one(world, VIEWPORT_COMPONENT,
                                                        viewport_ent, false);

    if (viewport) {
        ct_rg_builder_t0 *builder;
        builder = ct_renderer_a0->viewport_builder(viewport->viewport);
        ct_renderer_a0->viewport_set_size(viewport->viewport, size);

        bgfx_texture_handle_t th;
        th = builder->get_texture(builder, RG_OUTPUT_TEXTURE);

        ct_debugui_a0->Image(th,
                             &size,
                             &CE_VEC4_ONE,
                             &CE_VEC4_ZERO);

    }
}

static const char *dock_title(uint64_t content,
                              uint64_t selected_object) {
    return ICON_FA_GAMEPAD " " WINDOW_NAME;
}

static const char *name() {
    return "game_view";
}

static uint64_t dock_flags() {
    return DebugUIWindowFlags_NoNavInputs |
           DebugUIWindowFlags_NoScrollbar |
           DebugUIWindowFlags_NoScrollWithMouse;
}

static struct ct_dock_i0 dock_api = {
        .type =  CT_GAME_I,
        .ui_flags = dock_flags,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .draw_menu = on_menu,
};


void CE_MODULE_LOAD(game_view)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_id_a0);

    api->add_impl(CT_DOCK_I, &dock_api, sizeof(dock_api));

    ct_dock_a0->create_dock(CT_GAME_I, true);
}

void CE_MODULE_UNLOAD(game_view)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
