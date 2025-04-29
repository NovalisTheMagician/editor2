#include "texture_collection.h"

#include "re.h"
#include "utils/string.h"

#if defined(_DEBUG)
#pragma push_macro("malloc")
#undef malloc
#pragma push_macro("calloc")
#undef calloc
#pragma push_macro("free")
#undef free
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#if defined(_DEBUG)
#pragma pop_macro("malloc")
#pragma pop_macro("calloc")
#pragma pop_macro("free")
#endif

#include <tgmath.h>

#include "utils.h"
#include "utils/hash.h"
#include "memory.h" // IWYU pragma: keep

static size_t Partition(Texture **arr, size_t lo, size_t hi)
{
    size_t pivIdx = floor((hi - lo) / 2.0f) + lo;
    Texture *pivot = arr[pivIdx];
    size_t i = lo - 1;
    size_t j = hi + 1;

    while(true)
    {
        do { i++; } while(strcmp(arr[i]->name, pivot->name) < 0);
        do { j--; } while(strcmp(arr[j]->name, pivot->name) > 0);

        if(i >= j) return j;

        Texture *a = arr[i];
        Texture *b = arr[j];
        a->orderIdx = j;
        b->orderIdx = i;
        arr[j] = a;
        arr[i] = b;
    }
}

static void Quicksort(Texture **arr, size_t lo, size_t hi)
{
    if(lo < hi)
    {
        size_t p = Partition(arr, lo, hi);
        Quicksort(arr, lo, p);
        Quicksort(arr, p+1, hi);
    }
}

void tc_init(TextureCollection *tc)
{
    tc->slots = calloc(NUM_SLOTS, sizeof *tc->slots);
    tc->order = calloc(NUM_BUCKETS * NUM_SLOTS, sizeof *tc->order);
}

void tc_destroy(TextureCollection *tc)
{
    free(tc->slots);
    free(tc->order);
}

static GLuint createTexture(int width, int height, const uint8_t *pixels)
{
    size_t numMipLevels = log2(max(width, height)) + 1;

    GLuint texId;
    glCreateTextures(GL_TEXTURE_2D, 1, &texId);
    glTextureStorage2D(texId, numMipLevels, GL_RGBA8, width, height);
    glTextureSubImage2D(texId, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateTextureMipmap(texId);
    glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texId;
}

bool tc_load(TextureCollection *tc, const char *name, const char *path, time_t mtime)
{
    Texture *existing = tc_get(tc, name);

    if(existing && existing->modTime >= mtime) return true;

    int width, height, comp;
    uint8_t *pixels = stbi_load(path, &width, &height, &comp, 4);
    if(!pixels) return false;

    GLuint texId = createTexture(width, height, pixels);

    stbi_image_free(pixels);

    if(existing)
    {
        glDeleteTextures(1, &existing->texture1);
        existing->texture1 = texId;
        existing->width = width;
        existing->height = height;
        existing->flags = TF_NONE;
        existing->modTime = mtime;

        free(existing->name);
        existing->name = CopyString(name);
    }
    else
    {
        uint64_t nameHash = hash(name) % NUM_SLOTS;
        size_t size = tc->slots[nameHash].size++;
        assert(size < NUM_BUCKETS);
        Texture *texture = &tc->slots[nameHash].textures[size];

        texture->texture1 = texId;
        texture->width = width;
        texture->height = height;
        texture->flags = TF_NONE;
        texture->name = CopyString(name);
        texture->modTime = mtime;

        size_t orderIdx = tc->size++;
        texture->orderIdx = orderIdx;

        tc->order[orderIdx] = texture;
    }

    return true;
}

bool tc_load_mem(TextureCollection *tc, const char *name, uint8_t *data, size_t dataSize, time_t mtime)
{
    Texture *existing = tc_get(tc, name);

    if(existing && existing->modTime >= mtime) return true;

    int width, height, comp;
    uint8_t *pixels = stbi_load_from_memory(data, dataSize, &width, &height, &comp, 4);
    if(!pixels) return false;

    GLuint texId = createTexture(width, height, pixels);

    stbi_image_free(pixels);

    if(existing)
    {
        glDeleteTextures(1, &existing->texture1);
        existing->texture1 = texId;
        existing->width = width;
        existing->height = height;
        existing->flags = TF_NONE;
        existing->modTime = mtime;

        free(existing->name);
        existing->name = CopyString(name);
    }
    else
    {
        uint64_t nameHash = hash(name) % NUM_SLOTS;
        size_t size = tc->slots[nameHash].size++;
        assert(size < NUM_BUCKETS);
        Texture *texture = &tc->slots[nameHash].textures[size];

        texture->texture1 = texId;
        texture->width = width;
        texture->height = height;
        texture->flags = TF_NONE;
        texture->name = CopyString(name);
        texture->modTime = mtime;

        size_t orderIdx = tc->size++;
        texture->orderIdx = orderIdx;

        tc->order[orderIdx] = texture;
    }

    return true;
}

void tc_iterate(const TextureCollection *tc, tc_itearte_cb cb, void *user)
{
    for(size_t i = 0; i < tc->size; ++i)
    {
        Texture *texture = tc->order[i];
        cb(texture, i, user);
    }
}

void tc_iterate_filter(const TextureCollection *tc, tc_itearte_cb cb, const char *filter, void *user)
{
    struct regex_t *regex = re_compile(filter);

    for(size_t i = 0, j = 0; i < tc->size; ++i)
    {
        Texture *texture = tc->order[i];
        int matchLen;
        int match = re_matchp(regex, texture->name, &matchLen);
        if(match != -1)
            cb(texture, j++, user);
    }
}

void tc_unload(TextureCollection *tc, const char *name)
{
    if(name == NULL) return;
    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;

    for(size_t i = 0; i < size; ++i)
    {
        Texture *texture = &tc->slots[nameHash].textures[i];
        if(strcmp(name, texture->name) == 0)
        {
            glDeleteTextures(1, &texture->texture1);
            free(texture->name);

            if(i < NUM_BUCKETS - 1)
                memmove(tc->slots[nameHash].textures + i, tc->slots[nameHash].textures + i + 1, size - 1 - i);

            tc->slots[nameHash].size--;

            memmove(tc->order + texture->orderIdx, tc->order + texture->orderIdx + 1, tc->size - 1 - texture->orderIdx);
            tc->size--;

            return;
        }
    }
}

void tc_unload_all(TextureCollection *tc)
{
    if(tc->size == 0) return;

    for(size_t i = 0; i < tc->size; ++i)
    {
        Texture *texture = tc->order[i];
        glDeleteTextures(1, &texture->texture1);
        free(texture->name);
    }

    for(size_t i = 0; i < NUM_SLOTS; ++i)
    {
        tc->slots[i].size = 0;
    }

    tc->size = 0;
}

bool tc_has(const TextureCollection *tc, const char *name)
{
    if(name == NULL) return false;
    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;

    for(size_t i = 0; i < size; ++i)
    {
        if(strcmp(name, tc->slots[nameHash].textures[i].name) == 0) return true;
    }

    return false;
}

Texture* tc_get(const TextureCollection *tc, const char *name)
{
    if(name == NULL) return NULL;
    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;

    for(size_t i = 0; i < size; ++i)
    {
        Texture *texture = &tc->slots[nameHash].textures[i];
        if(strcmp(name, texture->name) == 0) return texture;
    }

    return NULL;
}

bool tc_set(TextureCollection *tc, const char *name, Texture texture)
{
    if(name == NULL) return false;
    Texture *existing = tc_get(tc, name);
    if(existing)
    {
        size_t orderNum = existing->orderIdx;
        free(existing->name);
        *existing = texture;
        existing->orderIdx = orderNum;
        existing->name = CopyString(name);

        return false;
    }

    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size++;
    assert(size < NUM_BUCKETS);
    Texture *tex = &tc->slots[nameHash].textures[size];

    *tex = texture;
    tex->name = CopyString(name);

    size_t orderIdx = tc->size++;
    tex->orderIdx = orderIdx;

    tc->order[orderIdx] = tex;

    return true;
}

size_t tc_size(TextureCollection *tc)
{
    return tc->size;
}

void tc_sort(TextureCollection *tc)
{
    if(tc->size > 0)
        Quicksort(tc->order, 0, tc->size-1);
}

bool tc_is_newer(const TextureCollection *tc, const char *name, time_t newTime)
{
    Texture *texture = tc_get(tc, name);
    if(texture)
    {
        return texture->modTime < newTime;
    }
    return true;
}
