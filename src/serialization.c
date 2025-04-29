#include "serialization.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char* Ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char* Rtrim(char *s)
{
    size_t len = strlen(s);
    if(len == 0) return s;
    char* back = s + len;
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char* Trim(char *s)
{
    if(!s) return NULL;
    return Rtrim(Ltrim(s));
}

char* ParseLineIndex(char *line, size_t *idx)
{
    char *delim = strchr(line, ' ');
    if(!delim) // end of line
        delim = line;
    else
        *delim = '\0';
    char *end;
    *idx = strtoull(line, &end, 10);
    if(line == end)
    {
        return NULL;
    }
    return delim == line ? NULL : delim+1;
}

char* ParseLineUint(char *line, uint32_t *i)
{
    char *delim = strchr(line, ' ');
    if(!delim) // end of line
        delim = line;
    else
        *delim = '\0';
    char *end;
    *i = strtoul(line, &end, 10);
    if(line == end)
    {
        return NULL;
    }
    return delim == line ? NULL : delim+1;
}

char* ParseLineInt(char *line, int *i)
{
    char *delim = strchr(line, ' ');
    if(!delim) // end of line
        delim = line;
    else
        *delim = '\0';
    char *end;
    *i = strtol(line, &end, 10);
    if(line == end)
    {
        return NULL;
    }
    return delim == line ? NULL : delim+1;
}

char* ParseLineFloat(char *line, float *f)
{
    char *delim = strchr(line, ' ');
    if(!delim) // end of line
        delim = line;
    else
        *delim = '\0';
    char *end;
    *f = strtof(line, &end);
    if(line == end)
    {
        return NULL;
    }
    return delim == line ? NULL : delim+1;
}

char* ParseLineString(char *line, char **str)
{
    char *delim = strchr(line, ' ');
    if(!delim) // end of line
        delim = line;
    else
        *delim = '\0';
    *str = line;
    return delim == line ? NULL : delim+1;
}

char* ParseLineTexture(char *line, char **texture)
{
    line = ParseLineString(line, texture);
    if(strcmp(*texture, "NULL") == 0)
        *texture = NULL;
    return line;
}

bool ParseBool(char *str, bool *val)
{
    char *end;
    *val = strtol(str, &end, 10);
    return !(str == end);
}

bool ParseInt(char *str, int *val)
{
    char *end;
    *val = strtol(str, &end, 10);
    return !(str == end);
}

bool ParseUint(char *str, uint32_t *val)
{
    char *end;
    *val = strtoul(str, &end, 10);
    return !(str == end);
}

bool ParseFloat(char *str, float *val)
{
    char *end;
    *val = strtof(str, &end);
    return !(str == end);
}

bool ParseVec4(char *str, vec4s *val)
{
    char *sep1 = strchr(str, ',');
    if(!sep1) return false;
    char *sep2 = strchr(sep1+1, ',');
    if(!sep2) return false;
    char *sep3 = strchr(sep2+1, ',');
    if(!sep3) return false;

    *sep1 = '\0';
    *sep2 = '\0';
    *sep3 = '\0';

    if(!ParseFloat(str, &val->x)) return false;
    if(!ParseFloat(sep1+1, &val->y)) return false;
    if(!ParseFloat(sep2+1, &val->z)) return false;
    if(!ParseFloat(sep3+1, &val->w)) return false;
    return true;
}
