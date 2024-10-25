#pragma once

#include <ftplib.h>
#include <stddef.h>

#include "texture_collection.h"
#include "async_load.h"

netbuf* ConnectToFtp(const char *url, const char *user, const char *pass);
size_t CollectTexturesFtp(TextureCollection *tc, FetchLocation **locations, size_t size, size_t *capacity, char *folder, char *baseFolder, netbuf *ftpHandle);
bool ReadFromFtp(const char *path, uint8_t **buffer, size_t *size, void *ftpHandle);
