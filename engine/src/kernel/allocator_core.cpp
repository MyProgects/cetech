#include <stdint.h>
#include <malloc.h>

#include <cetech/celib/allocator.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/memory.h>

static void *_reallocate(allocator_instance_v0 *a,
                         void *ptr,
                         uint32_t size,
                         uint32_t align) {
    void *new_ptr = NULL;

    if (size)
        new_ptr = realloc(ptr, size);

    else
        free(ptr);

    return new_ptr;
}


static allocator _allocator = {
        .inst = NULL,
        .reallocate= _reallocate
};


namespace core_allocator {
    allocator *get() {
        return &_allocator;
    }

    struct core_allocator_api_v0 core_allocator_api = {
        .get_allocator = core_allocator::get
    };

    void register_api(api_v0 *api) {
        api->register_api("core_allocator_api_v0", &core_allocator_api);
    }
}

