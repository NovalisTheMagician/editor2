#include "texture_collection.h"

#include <re.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <tgmath.h>

void tc_init(struct TextureCollection *tc)
{
    tc->slots = calloc(NUM_SLOTS, sizeof *tc->slots);
    tc->order = calloc(NUM_BUCKETS * NUM_SLOTS, sizeof *tc->order);
}

void tc_destroy(struct TextureCollection *tc)
{
    free(tc->slots);
    free(tc->order);
}

bool tc_load(struct TextureCollection *tc, pstring name, pstring path)
{
    struct Texture *existing = tc_get(tc, name);

    int width, height, comp;
    uint8_t *pixels = stbi_load(pstr_tocstr(path), &width, &height, &comp, 4);
    if(!pixels) return false;

    size_t numMipLevels = log2(max(width, height)) + 1;

    GLuint texId;
    glCreateTextures(1, GL_TEXTURE_2D, &texId);
    glTextureStorage2D(texId, numMipLevels, GL_RGBA8, width, height);
    glTextureSubImage2D(texId, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateTextureMipmap(texId);
    glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(pixels);

    if(existing)
    {
        glDeleteTextures(1, &existing->texture1);
        existing->texture1 = texId;
        existing->width = width;
        existing->height = height;
        existing->flags = TF_NONE;
    }
    else
    {
        uint64_t nameHash = hash(name) % NUM_SLOTS;
        size_t size = tc->slots[nameHash].size;
        assert(size < NUM_BUCKETS);
        struct Texture *texture = &tc->slots[nameHash].textures[size++];

        texture->texture1 = texId;
        texture->width = width;
        texture->height = height;
        texture->flags = TF_NONE;

        size_t orderIdx = tc->size++;
        texture->orderIdx = orderIdx;

        tc->order[orderIdx] = texture;
    }

    return true;
}

void tc_iterate(struct TextureCollection *tc, tc_itearte_cb cb, void *user)
{
    for(size_t i = 0; i < tc->size; ++i)
    {
        struct Texture *texture = tc->order[i];
        cb(texture, user);
    }
}

void tc_iterate_filter(struct TextureCollection *tc, tc_itearte_cb cb, pstring filter, void *user)
{
    struct regex_t *regex = re_compile(pstr_tocstr(filter));

    for(size_t i = 0; i < tc->size; ++i)
    {
        struct Texture *texture = tc->order[i];
        int matchLen;
        int match = re_matchp(regex, pstr_tocstr(texture->name), &matchLen);
        if(match != -1)
            cb(texture, user);
    }
}

void tc_unload(struct TextureCollection *tc, pstring name)
{
    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;

    for(size_t i = 0; i < size; ++i)
    {
        struct Texture *texture = &tc->slots[nameHash].textures[i];
        if(pstr_cmp(name, texture->name) == 0)
        {
            glDeleteTextures(1, &texture->texture1);

            if(i < NUM_BUCKETS - 1)
                memmove(tc->slots[nameHash].textures + i, tc->slots[nameHash].textures + i + 1, size - 1 - i);

            tc->slots[nameHash].size--;

            memmove(tc->order + texture->orderIdx, tc->order + texture->orderIdx + 1, tc->size - 1 - texture->orderIdx);
            tc->size--;

            return;
        }
    }
}

void tc_unload_all(struct TextureCollection *tc)
{
    if(tc->size == 0) return;

    for(size_t i = 0; i < tc->size; ++i)
    {
        struct Texture *texture = tc->order[i];
        glDeleteTextures(1, &texture->texture1);
    }

    for(size_t i = 0; i < NUM_SLOTS; ++i)
    {
        tc->slots[i].size = 0;
    }

    tc->size = 0;
}

bool tc_has(struct TextureCollection *tc, pstring name)
{
    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;

    for(size_t i = 0; i < size; ++i)
    {
        if(pstr_cmp(name, tc->slots[nameHash].textures[i].name) == 0) return true;
    }

    return false;
}

struct Texture* tc_get(struct TextureCollection *tc, pstring name)
{
    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;

    for(size_t i = 0; i < size; ++i)
    {
        struct Texture *texture = &tc->slots[nameHash].textures[i];
        if(pstr_cmp(name, texture->name) == 0) return texture;
    }

    return NULL;
}

bool tc_set(struct TextureCollection *tc, pstring name, struct Texture texture)
{
    struct Texture *existing = tc_get(tc, name);
    if(existing)
    {
        size_t orderNum = existing->orderIdx;
        *existing = texture;
        existing->orderIdx = orderNum;

        return false;
    }

    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size;
    assert(size < NUM_BUCKETS);
    struct Texture *tex = &tc->slots[nameHash].textures[size++];

    *tex = texture;

    size_t orderIdx = tc->size++;
    tex->orderIdx = orderIdx;

    tc->order[orderIdx] = tex;

    return true;
}

size_t tc_size(struct TextureCollection *tc)
{
    return tc->size;
}
