#pragma once

#include <stddef.h>

#include "../texture_collection.h"
#include "../async_load.h"

size_t CollectTexturesFs(TextureCollection *tc, FetchLocation **locations, size_t size, size_t *capacity, char *folder, char *baseFolder);
bool ReadFromFs(const char *path, uint8_t **buffer, size_t *size, void *unused);
