#pragma once

#include "glad/gl.h"

#include "memory.h"

typedef enum TextureFlags
{
    TF_NONE,
    TF_SKY
} TextureFlags;

typedef struct Texture
{
    pstring name;
    int width, height;
    TextureFlags flags;
    GLuint texture1, texture2;
    GLuint64 textureHandle;
    size_t orderIdx;
    time_t modTime;
    ssize_t activeIndex;
} Texture;

#define NUM_BUCKETS 128
#define NUM_SLOTS 4096

typedef struct TextureCollection
{
    struct
    {
        Texture textures[NUM_BUCKETS];
        size_t size;
    } *slots;

    Texture **order;
    size_t size;
    Texture **activeSet;
    size_t numActive;
} TextureCollection;

typedef void (*tc_itearte_cb)(Texture *texture, size_t idx, void *user);

void tc_init(TextureCollection *tc);
void tc_destroy(TextureCollection *tc);
bool tc_load(TextureCollection *tc, pstring name, pstring path, time_t mtime);
bool tc_load_mem(TextureCollection *tc, pstring name, uint8_t *data, size_t dataSize, time_t mtime);
void tc_iterate(TextureCollection *tc, tc_itearte_cb cb, void *user);
void tc_iterate_filter(TextureCollection *tc, tc_itearte_cb cb, pstring filter, void *user);
void tc_iterate_active(TextureCollection *tc, tc_itearte_cb cb, void *user);
void tc_unload(TextureCollection *tc, pstring name);
void tc_unload_all(TextureCollection *tc);
bool tc_has(TextureCollection *tc, pstring name);
Texture* tc_get(TextureCollection *tc, pstring name);
bool tc_set(TextureCollection *tc, pstring name, Texture texture);
size_t tc_size(TextureCollection *tc);
void tc_sort(TextureCollection *tc);
bool tc_is_newer(TextureCollection *tc, pstring name, time_t newTime);
void tc_active(TextureCollection *tc, Texture *texture);
void tc_inactive(TextureCollection *tc, Texture *texture);
