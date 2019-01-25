#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/hashlib.h>
#include <celib/api_system.h>
#include <celib/cdb.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/renderer/gfx.h>
#include <cetech/resource/resource.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/explorer.h>
#include <stdio.h>
#include <cetech/node_graph_editor/node_graph_editor.h>
#include <cetech/editor/resource_preview.h>

#include "../node_graph.h"

#define _G node_graph_globals
static struct _G {
    struct ce_alloc *allocator;
} _G;

static void create_new(uint64_t obj) {
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);

    if (!ce_cdb_a0->prop_exist(w, CT_NODE_GRAPH_NODES)) {
        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                               CT_NODE_GRAPH_NODES);
        ce_cdb_a0->set_subobject(w, CT_NODE_GRAPH_NODES, ch);
    }

    if (!ce_cdb_a0->prop_exist(w, CT_NODE_GRAPH_CONNECTIONS)) {
        uint64_t ch = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                               CT_NODE_GRAPH_CONNECTIONS);
        ce_cdb_a0->set_subobject(w, CT_NODE_GRAPH_CONNECTIONS, ch);
    }

    ce_cdb_a0->write_commit(w);
}

static const char *display_icon() {
    return ICON_FA_CUBES;
}

static uint64_t cdb_type() {
    return CT_NODE_GRAPH_RESOURCE;
}


void draw_raw(uint64_t obj, float size[2]) {
    ct_node_graph_editor_a0->draw_ng_editor(obj);
}

static struct ct_resource_preview_i0 resource_preview_i0 = {
        .draw_raw = draw_raw,
};

static void *get_res_interface(uint64_t name_hash) {
    if (name_hash == RESOURCE_PREVIEW_I) {
        return &resource_preview_i0;
    }
    return NULL;
}

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .create_new = create_new,
        .get_interface= get_res_interface,
};


static uint64_t draw_ui(uint64_t top_level_obj,
                        uint64_t selected_obj,
                        uint64_t context) {
    if (!top_level_obj) {
        return 0;
    }

    if (!selected_obj) {
        return 0;
    }

    ct_debugui_a0->Columns(2, NULL, true);

    ct_debugui_a0->NextColumn();
    ct_debugui_a0->NextColumn();


    //

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                 top_level_obj);

    ImGuiTreeNodeFlags flags = 0 |
                               DebugUITreeNodeFlags_OpenOnArrow |
                               //                               DebugUITreeNodeFlags_OpenOnDoubleClick |
                               //                               DebugUITreeNodeFlags_DefaultOpen;
                               0;
    uint64_t new_selected_object = 0;


    uint64_t nodes = ce_cdb_a0->read_subobject(reader, CT_NODE_GRAPH_NODES, 0);
    const ce_cdb_obj_o *ns_reader = ce_cdb_a0->read(ce_cdb_a0->db(), nodes);

    const uint64_t ns_n = ce_cdb_a0->prop_count(ns_reader);

    if (!ns_n) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }

    char name[128] = {0};
    uint64_t uid = top_level_obj;
    snprintf(name, CE_ARRAY_LEN(name), "Nodes");

    bool selected = selected_obj == nodes;
    if (selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }

    char label[128] = {0};

    //menu
    snprintf(label, CE_ARRAY_LEN(label), ICON_FA_PLUS
            "##add_%llu", nodes);

    bool add = ct_debugui_a0->Button(label, (float[2]) {0.0f});

    char modal_id[128] = {'\0'};
    sprintf(modal_id, "select...##select_node_%llu", uid);
    ct_node_graph_editor_a0->add_node_modal(modal_id, uid);

    if (add) {
        ct_debugui_a0->OpenPopup(modal_id);
    }
    ct_debugui_a0->NextColumn();


    snprintf(label, CE_ARRAY_LEN(label),
             (ICON_FA_CUBE
                     " ""%s##nodes_%llu"), name, uid);
    const bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        new_selected_object = nodes;
    }

    ct_debugui_a0->NextColumn();

    ct_debugui_a0->NextColumn();
    if (open) {
        const uint64_t *ns = ce_cdb_a0->prop_keys(ns_reader);
        for (int i = 0; i < ns_n; ++i) {
            uint64_t node = ns[i];
            snprintf(label, CE_ARRAY_LEN(label),
                     (ICON_FA_CUBE
                             " ""%llx##node%llu"), node, node);

            flags = DebugUITreeNodeFlags_Leaf;

            if (selected_obj == node) {
                flags |= DebugUITreeNodeFlags_Selected;
            }

            if (ct_debugui_a0->TreeNodeEx(label, flags)) {
                if (ct_debugui_a0->IsItemClicked(0)) {
                    new_selected_object = node;
                }
                ct_debugui_a0->TreePop();
            }
        }

        ct_debugui_a0->TreePop();
    }

    ct_debugui_a0->NextColumn();

    ct_debugui_a0->Columns(1, NULL, true);

    return new_selected_object;
}

static void draw_menu(uint64_t selected_obj,
                      uint64_t context) {
    if (!selected_obj) {
        return;
    }

}

static struct ct_node_i0 *get_interface(uint64_t type) {
    struct ce_api_entry it = ce_api_a0->first(CT_NODE_I);

    while (it.api) {
        struct ct_node_i0 *i = (it.api);

        if (i && i->cdb_type && (i->cdb_type() == type)) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

static struct ct_node_graph_a0 ng_api = {
        .get_interface = get_interface,
};

struct ct_node_graph_a0 *ct_node_graph_a0 = &ng_api;


void CE_MODULE_INITAPI(node_graph)(struct ce_api_a0 *api) {

}

void CE_MODULE_LOAD (node_graph)(struct ce_api_a0 *api,
                                 int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_id_a0->id64("node_graph");
    ce_id_a0->id64("nodes");
    ce_id_a0->id64("connections");
    ce_id_a0->id64("position_x");
    ce_id_a0->id64("position_y");

    api->register_api(CT_NODE_GRAPH_API, ct_node_graph_a0);
    api->register_api(RESOURCE_I, &ct_resource_i0);

    static struct ct_explorer_i0 entity_explorer = {
            .cdb_type = cdb_type,
            .draw_ui = draw_ui,
            .draw_menu = draw_menu,
    };

    api->register_api(EXPLORER_INTERFACE, &entity_explorer);

}

void CE_MODULE_UNLOAD (node_graph)(struct ce_api_a0 *api,
                                   int reload) {

    CE_UNUSED(api);
}