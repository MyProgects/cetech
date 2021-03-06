#ifndef CETECH_GAME_H
#define CETECH_GAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_GAME_SYSTEM_API \
    CE_ID64_0("ct_game_system_a0", 0x1a5b2ed4808612b9ULL)

#define CT_GAME_I \
    CE_ID64_0("ct_game_i0", 0x22500b95a05f8b37ULL)

#define CT_GAME_TASK \
    CE_ID64_0("game_task", 0x56c73acaa12b1278ULL)

typedef struct ct_rg_builder_t0 ct_rg_builder_t0;
typedef struct ct_viewport_t0 ct_viewport_t0;

typedef struct ct_game_i0 {
    uint64_t (*name)();

    void (*init)(ct_world_t0 world);

    void (*shutdown)(ct_world_t0 world);

    void (*update)(ct_world_t0 world,
                   float dt);
} ct_game_i0;

struct ct_game_system_a0 {
    ct_world_t0 (*world)();

    void (*pause)();

    bool (*is_paused)();

    void (*play)();

    void (*step)(float dt);
};

CE_MODULE(ct_game_system_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_GAME_H
