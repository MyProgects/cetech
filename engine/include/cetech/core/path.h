#ifndef CETECH_PATH_H
#define CETECH_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

struct allocator;

//==============================================================================
// File Interface
//==============================================================================

struct path_v0 {
    //! Get file modified time
    //! \param path File path
    //! \return Modified time
    uint32_t (*file_mtime)(const char *path);

    //! List dir
    //! \param path Dir path
    //! \param recursive Resucrsive list?
    //! \param files Result files
    //! \param allocator Allocator
    void (*list)(const char *path,
                 int recursive,
                 char ***files,
                 uint32_t *count,
                 struct allocator *allocator);

    //! Free list dir array
    //! \param files Files array
    //! \param allocator Allocator
    void (*list_free)(char **files,
                      uint32_t count,
                      struct allocator *allocator);

    //! Create dir path
    //! \param path Path
    //! \return 1 of ok else 0
    int (*make_path)(const char *path);

    //! Get filename from path
    //! \param path Path
    //! \return Filename
    const char *(*filename)(const char *path);

    //! Get file basename (filename without extension)
    //! \param path Path
    //! \param out Out basename
    //! \param size
    void (*basename)(const char *path,
                     char *out,
                     size_t size);

    void (*dir)(char *out,
                size_t size,
                const char *path);

    //! Get file extension
    //! \param path Path
    //! \return file extension
    const char *(*extension)(const char *path);

    //! Join paths
    //! \param allocator Allocator
    //! \param count Path count.
    //! \return Result path len.
    char* (*join)(struct allocator* allocator,
                    uint32_t count,
                    ...);
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_PATH_H