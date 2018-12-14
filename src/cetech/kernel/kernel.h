#ifndef CETECH_KERNEL_H
#define CETECH_KERNEL_H

#define CONFIG_PLATFORM \
     CE_ID64_0("kernel.platform", 0xa2e788cf39b7339bULL)

#define CONFIG_NATIVE_PLATFORM \
     CE_ID64_0("kernel.native_platform", 0xede1e0cf393fc17eULL)

#define CONFIG_COMPILE \
     CE_ID64_0("compile", 0x3c797c340e1e5467ULL)

#define CONFIG_BOOT_PKG \
     CE_ID64_0("core.boot_pkg", 0xd065c28a0c45037eULL)

#define CONFIG_CONTINUE \
     CE_ID64_0("continue", 0xb3326d55cc9374b8ULL)

#define CONFIG_WAIT \
     CE_ID64_0("wait", 0x21778fd44dd2ab6aULL)

#define CONFIG_GAME \
     CE_ID64_0("game", 0x8a426cdbe2604989ULL)

#define CONFIG_DAEMON \
     CE_ID64_0("daemon", 0xc3b953e09c1d1f60ULL)

#define KERNEL_EVENT_DT \
    CE_ID64_0("dt", 0xbd04987fa96a9de5ULL)

#define SOURCE_ROOT \
    CE_ID64_0("source", 0x921f1370045bad6eULL)

#define BUILD_ROOT \
    CE_ID64_0("build", 0x4429661936ece1eaULL)

#define KERNEL_INIT_EVENT \
    CE_ID64_0("init", 0x44181d1a66341876ULL)

#define KERNEL_UPDATE_EVENT \
    CE_ID64_0("update", 0xb09bdf90e8a56b76ULL)

#define KERNEL_POST_UPDATE_EVENT \
    CE_ID64_0("gc", 0xf7e41039a96af360ULL)

#define KERNEL_QUIT_EVENT \
    CE_ID64_0("quit", 0x666ffde8529c8bc9ULL)

#define KERNEL_SHUTDOWN_EVENT \
    CE_ID64_0("shutdown", 0xe1649d1d8b13d056ULL)

#define KERNEL_EBUS \
    CE_ID64_0("kernel", 0x146c2a8f5727f34ULL)

enum {
    KERNEL_ORDER = 1024,
    GAME_ORDER = KERNEL_ORDER + 1024,
    RENDER_ORDER = GAME_ORDER + 1024,
};


#endif //CETECH_KERNEL_H
