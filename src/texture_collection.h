#pragma once

#include "common.h"

enum TextureFlags
{
    TF_NONE,
    TF_SKY
};

struct Texture
{
    bool loading;
    pstring name;
    int width, height;
    enum TextureFlags flags;
    GLuint texture1, texture2;
    size_t orderIdx;
};

#define NUM_BUCKETS 128
#define NUM_SLOTS 2048

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

typedef void (*tc_itearte_cb)(struct Texture *texture, void *user);

void tc_init(struct TextureCollection *tc);
void tc_destroy(struct TextureCollection *tc);
bool tc_load(struct TextureCollection *tc, pstring name, pstring path);
void tc_iterate(struct TextureCollection *tc, tc_itearte_cb cb, void *user);
void tc_iterate_filter(struct TextureCollection *tc, tc_itearte_cb cb, pstring filter, void *user);
void tc_unload(struct TextureCollection *tc, pstring name);
void tc_unload_all(struct TextureCollection *tc);
bool tc_has(struct TextureCollection *tc, pstring name);
struct Texture* tc_get(struct TextureCollection *tc, pstring name);
size_t tc_size(struct TextureCollection *tc);
