#pragma once

#include "common.h"
#include "project.h"
#include "async_load.h"

enum TextureFlags
{
    TF_NONE,
    TF_SKY
};

struct Texture
{
    pstring name;
    int width, height;
    enum TextureFlags flags;
    GLuint texture1, texture2;
    size_t orderIdx;
    time_t modTime;
};

#define NUM_BUCKETS 128
#define NUM_SLOTS 4096

struct TextureCollection
{
    struct 
    {
        struct Texture textures[NUM_BUCKETS];
        size_t size;
    } *slots;

    struct Texture **order;
    size_t size;
};

void LoadTextures(struct TextureCollection *tc, struct Project *project, struct AsyncJob *async, bool refresh);

typedef void (*tc_itearte_cb)(struct Texture *texture, size_t idx, void *user);

void tc_init(struct TextureCollection *tc);
void tc_destroy(struct TextureCollection *tc);
bool tc_load(struct TextureCollection *tc, pstring name, pstring path, time_t mtime);
bool tc_load_mem(struct TextureCollection *tc, pstring name, uint8_t *data, size_t dataSize, time_t mtime);
void tc_iterate(struct TextureCollection *tc, tc_itearte_cb cb, void *user);
void tc_iterate_filter(struct TextureCollection *tc, tc_itearte_cb cb, pstring filter, void *user);
void tc_unload(struct TextureCollection *tc, pstring name);
void tc_unload_all(struct TextureCollection *tc);
bool tc_has(struct TextureCollection *tc, pstring name);
struct Texture* tc_get(struct TextureCollection *tc, pstring name);
bool tc_set(struct TextureCollection *tc, pstring name, struct Texture texture);
size_t tc_size(struct TextureCollection *tc);
void tc_sort(struct TextureCollection *tc);
bool tc_is_newer(struct TextureCollection *tc, pstring name, time_t newTime);
