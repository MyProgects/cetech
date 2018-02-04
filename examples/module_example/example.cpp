#include <cetech/core/macros.h>

#include <cetech/core/log/log.h>
#include <cetech/core/config/config.h>
#include <cetech/core/module/module.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/hashlib/hashlib.h>

#include <cetech/engine/input/input.h>
#include <cetech/engine/application/application.h>
#include <cetech/engine/entity/entity.h>

#include <cetech/playground//playground.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/engine/level/level.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/engine/renderer/texture.h>
#include <cstdlib>

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_hashlib_a0);

CETECH_DECL_API(ct_renderer_a0);

CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_level_a0);
CETECH_DECL_API(ct_texture_a0);


static struct G {
    ct_viewport viewport;
    ct_world world;
    ct_entity camera_ent;
    float dt;
} _G;


void update(float dt) {
    _G.dt = dt;

    if (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index("v"))) {
        ct_log_a0.info("example", "PO");
        ct_log_a0.error("example", "LICE");
    }
    ///ct_log_a0.debug("example", "%f", dt);
}

void module1() {
    static bool visible = true;
    if (ct_debugui_a0.BeginDock("Module 1", &visible,
                                DebugUIWindowFlags_Empty)) {

        ct_debugui_a0.Text("DT: %f", _G.dt);
        ct_debugui_a0.Text("FPS: %f", 1.0f / _G.dt );
        ct_debugui_a0.Text("Raddnddddom FPS: %f", static_cast<double>(rand()));

        float fps =  (1.0f / _G.dt);

        ct_debugui_a0.PlotLines("FPS", &fps, 1, 0, NULL, 0.0f, 0.0f, (float[2]){100.0f, 50.0f}, 0);

        ct_debugui_a0.Text("xknalsnxlsanlknxlasnlknxslknsaxdear imgui, %d", 111);

            static float v[2] = {100.0f, 100.0f};
            if(ct_debugui_a0.Button("sjdoiasjwww", v)) {
                ct_log_a0.debug("dasdsa", "dsadsdsadsadsadasd");
            }

        static float col[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        ct_debugui_a0.ColorButton(col, 1, 2);

        static float vv;
        ct_debugui_a0.DragFloat("FOO:", &vv, 1.0f, 0.0f, 10000.0f, "%.3f",
                                1.0f);

        static float col2[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        ct_debugui_a0.ColorEdit3("COLOR", col2);

        static float col3[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        ct_debugui_a0.ColorWheel("WHEEE", col3, 0.2f);


        float size[2] = {};
        ct_debugui_a0.GetWindowSize(size);
        size[1] = size[0];
        ct_debugui_a0.Image2(ct_texture_a0.get(ct_hashlib_a0.id64_from_str("content/scene/duck/duckCM")),//"content/scene/m4a1/m4_diff"
                             size,
                             (float[2]) {0.0f, 0.0f},
                             (float[2]) {1.0f, 1.0f},
                             (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                             (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
    }
    ct_debugui_a0.EndDock();

}

void module2() {
    static bool visible = true;
    if (ct_debugui_a0.BeginDock("Module 2", &visible,
                                DebugUIWindowFlags_Empty)) {
        ct_debugui_a0.Text("dear imgui, %d", 111);
        ct_debugui_a0.Text("By Omar Cornut and all github contributors.");
        ct_debugui_a0.Text(
                "ImGui is licensed under the MIT License, see LICENSE for more information.");
    }
    ct_debugui_a0.EndDock();
}



//==============================================================================
// Module def
//==============================================================================
CETECH_MODULE_DEF(
        example,

//==============================================================================
// Init api
//==============================================================================
        {
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_renderer_a0);

            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_level_a0);
            CETECH_GET_API(api, ct_texture_a0);
        },

//==============================================================================
// Load
//==============================================================================
        {
            CT_UNUSED(api);

            ct_log_a0.info("example", "Init %d", reload);

            ct_app_a0.register_on_update(update);

            ct_debugui_a0.register_on_debugui(module1);
            //ct_debugui_a0.register_on_debugui(module2);
        },

//==============================================================================
// Unload
//==============================================================================
        {
            CT_UNUSED(api);

            ct_log_a0.info("example", "Shutdown %d", reload);

            ct_app_a0.unregister_on_update(update);

            ct_debugui_a0.unregister_on_debugui(module1);
            ct_debugui_a0.unregister_on_debugui(module2);
        }
)


