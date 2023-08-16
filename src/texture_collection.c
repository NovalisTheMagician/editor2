#include "texture_collection.h"

#include <re.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <tgmath.h>

#include "editor.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <ftplib.h>

static void LoadFolder(struct TextureCollection *tc, pstring folder, pstring baseFolder)
{
    DIR *dp = opendir(pstr_tocstr(folder));
    if(!dp)
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dp)))
    {
        pstring fileName = pstr_cstr(entry->d_name);
        if(pstr_cmp(fileName, ".") == 0 || pstr_cmp(fileName, "..") == 0) continue;

        pstring filePath = pstr_alloc(256);
        pstr_format(&filePath, "{s}/{s}", folder, fileName);

        struct stat buf;
        int r = stat(pstr_tocstr(filePath), &buf);
        if(r == -1)
        {
            perror("stat");
            return;
        }

        if(S_ISDIR(buf.st_mode))
        {
            LoadFolder(tc, filePath, baseFolder);
        }
        else if(S_ISREG(buf.st_mode))
        {
            ssize_t extIdx = pstr_last_index_of(filePath, ".");
            if(extIdx != -1)
            {
                pstring name = pstr_substring(filePath, baseFolder.size+1, extIdx);
                if(!tc_load(tc, name, filePath, buf.st_mtime))
                {
                    printf("failed to load texture: %s\n", pstr_tocstr(filePath));
                }
            }
        }

        pstr_free(fileName);
        pstr_free(filePath);
    }

    closedir(dp);
}

static void LoadFtpFolder(struct TextureCollection *tc, netbuf *ftpHandle, pstring folder, pstring baseFolder)
{
    netbuf *dirHandle;
    if(!FtpAccess(pstr_tocstr(folder), FTPLIB_DIR_VERBOSE, FTPLIB_ASCII, ftpHandle, &dirHandle))
    {
        printf("%s:", FtpLastResponse(ftpHandle));
        printf("failed to get dir listing: %s\n", pstr_tocstr(folder));
        return;
    }

    struct 
    {
        pstring filePath;
        pstring fileName;
        bool isDir;
    } *files = calloc(4096, sizeof *files);

    pstring buffer = pstr_alloc(1024);
    size_t numFiles = 0;
    ssize_t readLen;
    while((readLen = FtpRead(buffer.data, buffer.capacity, dirHandle)) > 0)
    {
        buffer.size = readLen;
        pstring line = buffer;
        pstring perm = pstr_tok(&line, " ");
        pstr_tok(&line, " "); // inode refs
        pstr_tok(&line, " "); // user
        pstr_tok(&line, " "); // group
        pstr_tok(&line, " "); // size?
        pstr_tok(&line, " "); // day
        pstr_tok(&line, " "); // month
        pstr_tok(&line, " "); // time
        pstring fileName = pstr_tok(&line, " ");
        if(fileName.data[fileName.size - 1] == '\n') fileName.size--;
        pstring filePath = pstr_alloc(256);
        pstr_format(&filePath, "{s}/{s}", folder, fileName);

        bool isDir = perm.data[0] == 'd';

        files[numFiles++] = (__typeof__(*files)){ .fileName = fileName, .filePath = filePath, .isDir = isDir };
    }
    FtpClose(dirHandle);

    for(size_t i = 0; i < numFiles; ++i)
    {
        if(files[i].isDir)
        {
            LoadFtpFolder(tc, ftpHandle, files[i].filePath, baseFolder);
        }
        else
        {
            ssize_t extIdx = pstr_last_index_of(files[i].filePath, ".");
            if(extIdx == -1) continue;
            pstring name = pstr_substring(files[i].filePath, baseFolder.size+1, extIdx);

            char timeBuffer[128] = { 0 };
            if(!FtpModDate(pstr_tocstr(files[i].filePath), timeBuffer, sizeof timeBuffer, ftpHandle))
            {
                printf("%s", FtpLastResponse(ftpHandle));
                continue;
            }

            struct tm tm = { .tm_isdst = -1 };
            // 'YYYYMMDDHHMMSS'
#if defined(_WIN32)
            sscanf(timeBuffer, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
#else
            strptime(timeBuffer, "%Y%m%d%H%M%S", &tm);
#endif
            time_t timestamp = mktime(&tm);

            if(!tc_is_newer(tc, name, timestamp)) continue;

            uint32_t fileSize = 0;
            if(!FtpSize(pstr_tocstr(files[i].filePath), &fileSize, FTPLIB_BINARY, ftpHandle))
            {
                printf("%s", FtpLastResponse(ftpHandle));
                continue;
            }

            netbuf *fileHandle;
            if(!FtpAccess(pstr_tocstr(files[i].filePath), FTPLIB_FILE_READ, FTPLIB_BINARY, ftpHandle, &fileHandle))
            {
                printf("%s", FtpLastResponse(ftpHandle));
                continue;
            }

            uint8_t *fileBuffer = calloc(fileSize, sizeof *fileBuffer);
            size_t readTotal = 0, readCurrent;
            while((readCurrent = FtpRead(fileBuffer + readTotal, fileSize - readTotal, fileHandle)) > 0)
            {
                if(readCurrent == -1)
                {
                    printf("%s", FtpLastResponse(ftpHandle));
                    goto cleanup;
                }
                readTotal += readCurrent;
                //if(readTotal >= fileSize) break;
            }

            if(!tc_load_mem(tc, name, fileBuffer, fileSize, timestamp))
            {
                printf("failed to load texture: %s\n", pstr_tocstr(files[i].filePath));
            }

cleanup:
            free(fileBuffer);
            FtpClose(fileHandle);
        }
    }

    pstr_free(buffer);
    for(size_t i = 0; i < numFiles; ++i) pstr_free(files[i].filePath);
    free(files);
}

static void LoadFtp(struct TextureCollection *tc, pstring folder, pstring url, pstring user, pstring pass)
{
    netbuf *handle;
    if(!FtpConnect(pstr_tocstr(url), &handle))
    {
        printf("failed to connect to ftp server: %s\n", pstr_tocstr(url));
        return;
    }

    if(!FtpLogin(pstr_tocstr(user), pstr_tocstr(pass), handle))
    {
        printf("failed to login to ftp server: %s\n", pstr_tocstr(user));
        goto close_session;
    }

    /*
    if(!FtpChdir(pstr_tocstr(folder), handle))
    {
        printf("%s", FtpLastResponse(handle));
        printf("failed to change directory: %s\n", pstr_tocstr(folder));
        goto close_session;
    }
    */

    LoadFtpFolder(tc, handle, folder, folder);

close_session:
    FtpQuit(handle);
}

void LoadTextures(struct TextureCollection *tc, struct Project *project, bool refresh)
{
    if(!refresh)
        tc_unload_all(tc);

    pstring textureFolder = pstr_alloc(256);

    if(project->basePath.fs.path.size == 0)
        pstr_format(&textureFolder, "{s}", project->texturesPath);
    else
        pstr_format(&textureFolder, "{s}/{s}", project->basePath.fs.path, project->texturesPath);

    if(project->basePath.type == ASSPATH_FTP)
        LoadFtp(tc, textureFolder, project->basePath.ftp.url, project->basePath.ftp.login, project->basePath.ftp.password);
    else
        LoadFolder(tc, textureFolder, textureFolder);

    tc_sort(tc);
    pstr_free(textureFolder);
}

static size_t Partition(struct Texture **arr, size_t lo, size_t hi)
{
    size_t pivIdx = floor((hi - lo) / 2) + lo;
    struct Texture *pivot = arr[pivIdx];
    size_t i = lo - 1;
    size_t j = hi + 1;

    while(true)
    {
        do { i++; } while(pstr_cmp(arr[i]->name, pivot->name) < 0);
        do { j--; } while(pstr_cmp(arr[j]->name, pivot->name) > 0);

        if(i >= j) return j;

        struct Texture *a = arr[i];
        struct Texture *b = arr[j];
        a->orderIdx = j;
        b->orderIdx = i;
        arr[j] = a;
        arr[i] = b;
    }
}

static void Quicksort(struct Texture **arr, size_t lo, size_t hi)
{
    if(lo >= 0 && hi >= 0 && lo < hi)
    {
        size_t p = Partition(arr, lo, hi);
        Quicksort(arr, lo, p);
        Quicksort(arr, p+1, hi);
    }
}

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

bool tc_load(struct TextureCollection *tc, pstring name, pstring path, time_t mtime)
{
    struct Texture *existing = tc_get(tc, name);

    if(existing && existing->modTime >= mtime) return true;

    int width, height, comp;
    uint8_t *pixels = stbi_load(pstr_tocstr(path), &width, &height, &comp, 4);
    if(!pixels) return false;

    size_t numMipLevels = log2(max(width, height)) + 1;

    GLuint texId;
    glCreateTextures(GL_TEXTURE_2D, 1, &texId);
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
        existing->modTime = mtime;

        pstr_free(existing->name);
        existing->name = pstr_copy(name);
    }
    else
    {
        uint64_t nameHash = hash(name) % NUM_SLOTS;
        size_t size = tc->slots[nameHash].size++;
        assert(size < NUM_BUCKETS);
        struct Texture *texture = &tc->slots[nameHash].textures[size];

        texture->texture1 = texId;
        texture->width = width;
        texture->height = height;
        texture->flags = TF_NONE;
        texture->name = pstr_copy(name);
        texture->modTime = mtime;

        size_t orderIdx = tc->size++;
        texture->orderIdx = orderIdx;

        tc->order[orderIdx] = texture;
    }

    return true;
}

bool tc_load_mem(struct TextureCollection *tc, pstring name, uint8_t *data, size_t dataSize, time_t mtime)
{
    struct Texture *existing = tc_get(tc, name);

    if(existing && existing->modTime >= mtime) return true;

    int width, height, comp;
    uint8_t *pixels = stbi_load_from_memory(data, dataSize, &width, &height, &comp, 4);
    if(!pixels) return false;

    size_t numMipLevels = log2(max(width, height)) + 1;

    GLuint texId;
    glCreateTextures(GL_TEXTURE_2D, 1, &texId);
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
        existing->modTime = mtime;

        pstr_free(existing->name);
        existing->name = pstr_copy(name);
    }
    else
    {
        uint64_t nameHash = hash(name) % NUM_SLOTS;
        size_t size = tc->slots[nameHash].size++;
        assert(size < NUM_BUCKETS);
        struct Texture *texture = &tc->slots[nameHash].textures[size];

        texture->texture1 = texId;
        texture->width = width;
        texture->height = height;
        texture->flags = TF_NONE;
        texture->name = pstr_copy(name);
        texture->modTime = mtime;

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
        cb(texture, i, user);
    }
}

void tc_iterate_filter(struct TextureCollection *tc, tc_itearte_cb cb, pstring filter, void *user)
{
    struct regex_t *regex = re_compile(pstr_tocstr(filter));

    for(size_t i = 0, j = 0; i < tc->size; ++i)
    {
        struct Texture *texture = tc->order[i];
        int matchLen;
        int match = re_matchp(regex, pstr_tocstr(texture->name), &matchLen);
        if(match != -1)
            cb(texture, j++, user);
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
            pstr_free(texture->name);

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
        pstr_free(texture->name);
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
        pstr_free(existing->name);
        *existing = texture;
        existing->orderIdx = orderNum;
        existing->name = pstr_copy(name);

        return false;
    }

    uint64_t nameHash = hash(name) % NUM_SLOTS;
    size_t size = tc->slots[nameHash].size++;
    assert(size < NUM_BUCKETS);
    struct Texture *tex = &tc->slots[nameHash].textures[size];

    *tex = texture;
    existing->name = pstr_copy(name);

    size_t orderIdx = tc->size++;
    tex->orderIdx = orderIdx;

    tc->order[orderIdx] = tex;

    return true;
}

size_t tc_size(struct TextureCollection *tc)
{
    return tc->size;
}

void tc_sort(struct TextureCollection *tc)
{
    if(tc->size > 0)
        Quicksort(tc->order, 0, tc->size-1);
}

bool tc_is_newer(struct TextureCollection *tc, pstring name, time_t newTime)
{
    struct Texture *texture = tc_get(tc, name);
    if(texture)
    {
        return texture->modTime < newTime;
    }
    return true;
}
