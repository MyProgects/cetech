#include <float.h>
#include <stdio.h>

#include <celib/hashlib.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/ydb.h>
#include <celib/array.inl>
#include <celib/module.h>

#include <cetech/debugui/debugui.h>
#include <cetech/resource/resource.h>
#include <cetech/ecs/ecs.h>

#include <cetech/editor/property_editor.h>
#include <cetech/editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <celib/ebus.h>
#include <celib/fmath.inl>
#include <cetech/editor/command_system.h>
#include <celib/hash.inl>
#include <celib/ydb.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/sourcedb/sourcedb_ui.h>
#include <celib/cdb.h>
#include <celib/log.h>
#include <celib/buffer.inl>
#include <cetech/sourcedb/sourcedb.h>


#define _G entity_property_global

static struct _G {
    uint64_t active_entity;
    struct ct_entity top_entity;
    struct ct_world active_world;

    struct ce_hash_t components;

    struct ce_alloc *allocator;
    uint64_t obj;
} _G;

static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
    struct ce_api_entry it = ce_api_a0->first(COMPONENT_INTERFACE);
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
};

static void _generic_component_property(struct ct_resource_id rid,
                                        uint64_t obj,
                                        struct ct_component_i0 *ci,
                                        struct ct_editor_component_i0 *ec) {

    const struct ct_comp_prop_decs *comp_decs = ci->prop_desc();

    for (int i = 0; i < comp_decs->prop_n; ++i) {
        const struct ct_prop_decs *desc = &comp_decs->prop_decs[i];
        uint64_t prop = desc->name;

        enum ct_ecs_prop_type type = desc->type;

        const char *display_name = ec->prop_display_name(prop) ?: "";

        switch (type) {
            case ECS_PROP_FLOAT:
                ct_sourcedb_ui_a0->ui_float(rid, obj, prop, display_name, 0.0f,
                                          0.0f);
                break;
            case ECS_PROP_VEC3:
                ct_sourcedb_ui_a0->ui_vec3(rid, obj, prop, display_name, 0.0f,
                                         0.0f);
                break;
            default:
                break;
        }

    }
}

static void draw_component(struct ct_resource_id rid,
                           uint64_t obj) {
    uint64_t type = ce_cdb_a0->type(obj);

    struct ct_component_i0 *c = get_component_interface(type);


    if (!c || !c->get_interface) {
        return;
    }

    struct ct_editor_component_i0 *editor = c->get_interface(EDITOR_COMPONENT);

    ct_debugui_a0->Separator();
    bool open = ct_debugui_a0->TreeNodeEx(editor->display_name(),
                                          DebugUITreeNodeFlags_DefaultOpen);

    ct_debugui_a0->NextColumn();

    uint64_t parent = ce_cdb_a0->parent(obj);
    uint64_t comp_type = ce_cdb_a0->type(obj);

    if (ct_debugui_a0->Button(ICON_FA_MINUS, (float[2]) {0.0f})) {
        uint64_t *keys = NULL;
        ct_sourcedb_a0->collect_keys(rid, parent, &keys, _G.allocator);
        uint64_t keys_n = ce_array_size(keys);

        ct_sourcedb_a0->remove_prop(rid, keys, keys_n, comp_type);
        ce_array_free(keys, _G.allocator);
    }

    ct_debugui_a0->Separator();

    ct_debugui_a0->NextColumn();

    if (!open) {
        return;
    }

    if (!editor->property_editor) {
        _generic_component_property(rid, obj, c, editor);

    } else {
        editor->property_editor(rid, obj);
    }


    ct_debugui_a0->TreePop();

}

static void _entity_ui(struct ct_resource_id rid,
                       uint64_t obj) {
    bool open = ct_debugui_a0->TreeNodeEx(ICON_FA_CUBE" Entity",
                                          DebugUITreeNodeFlags_DefaultOpen);

    ct_debugui_a0->NextColumn();
    ct_debugui_a0->NextColumn();

    if (!open) {
        return;
    }

    char buffer[128] = {};
    snprintf(buffer, CE_ARRAY_LEN(buffer), "%llu", ce_cdb_a0->key(obj));

    ct_debugui_a0->Text("UID");
    ct_debugui_a0->NextColumn();
    ct_debugui_a0->PushItemWidth(-1);
    ct_debugui_a0->InputText("##EntityUID",
                             buffer,
                             strlen(buffer),
                             DebugInputTextFlags_ReadOnly,
                             0, NULL);
    ct_debugui_a0->PopItemWidth();
    ct_debugui_a0->NextColumn();

    ct_sourcedb_ui_a0->ui_str(rid, obj, ENTITY_NAME, "Name", rid.name);


    ct_debugui_a0->TreePop();
}

static void draw_ui(struct ct_resource_id rid,
                    uint64_t obj) {
    if (!obj) {
        return;
    }

    uint64_t obj_type = ce_cdb_a0->type(obj);

    if (ENTITY_RESOURCE_ID == obj_type) {
        _entity_ui(rid, obj);

        uint64_t components_obj;
        components_obj = ce_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);

        uint64_t n = ce_cdb_a0->prop_count(components_obj);
        uint64_t components_name[n];
        ce_cdb_a0->prop_keys(components_obj, components_name);

        for (uint64_t j = 0; j < n; ++j) {
            uint64_t name = components_name[j];

            uint64_t c_obj;
            c_obj = ce_cdb_a0->read_subobject(components_obj, name, 0);

            draw_component(rid, c_obj);
        }

    } else if (get_component_interface(obj_type)) {
        draw_component(rid, obj);
    }
}

void draw_menu(uint64_t obj) {
    if (!obj) {
        return;
    }

    uint64_t obj_type = ce_cdb_a0->type(obj);
    if ((ENTITY_RESOURCE_ID != obj_type)) {
        return;
    }

    struct ct_resource_id rid = {
            .name=ce_cdb_a0->read_uint64(ct_sourcedb_a0->find_root(obj),
                                         ASSET_NAME, 0),
            .type=obj_type,
    };


    ct_debugui_a0->Button(ICON_FA_PLUS" component", (float[2]) {0.0f});
    if (ct_debugui_a0->BeginPopupContextItem("add component context menu", 0)) {
        struct ce_api_entry it = ce_api_a0->first(COMPONENT_I);
        while (it.api) {
            struct ct_component_i0 *i = (it.api);
            struct ct_editor_component_i0 *ei;

            if (!i->get_interface) {
                goto next;
            }

            ei = i->get_interface(EDITOR_COMPONENT);

            uint64_t components;
            components = ce_cdb_a0->read_subobject(obj,
                                                   ENTITY_COMPONENTS,
                                                   0);

            uint64_t component_type = i->cdb_type();
            if (ei->display_name &&
                !ce_cdb_a0->prop_exist(components, component_type)) {
                const char *label = ei->display_name();
                bool add = ct_debugui_a0->Selectable(label, false, 0,
                                                     (float[2]) {0.0f});

                if (add) {
                    uint64_t component;
                    if (ei->create_new) {
                        component = ei->create_new();
                    } else {
                        component = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                             component_type);
                    }
                    uint64_t *keys = NULL;
                    ct_sourcedb_a0->collect_keys(rid, obj, &keys, _G.allocator);
                    ce_array_push(keys, ENTITY_COMPONENTS, _G.allocator);
                    uint64_t keys_n = ce_array_size(keys);

                    ct_sourcedb_a0->add_subobj(rid, component_type,
                                               keys, keys_n, 0, component);
                    ce_array_free(keys, _G.allocator);
                }
            }
            next:
            it = ce_api_a0->next(it);
        }

        ct_debugui_a0->EndPopup();
    }
}

static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .draw_ui = draw_ui,
        .draw_menu = draw_menu,
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        entity_property,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_cmd_system_a0);
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
