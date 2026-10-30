#ifndef SYSTEM_H_
#define SYSTEM_H_

#define combinePaths(...) combinePaths_(__VA_ARGS__, NULL)

const char* getPreferredFolder(const char *subpath);
const char* combinePaths_(const char *first, ...);

#endif

