#pragma once

#include <stdint.h>
#include <cglm/struct.h>

char* Ltrim(char *s);
char* Rtrim(char *s);
char* Trim(char *s);

char* ParseLineIndex(char *line, size_t *idx);
char* ParseLineUint(char *line, uint32_t *i);
char* ParseLineInt(char *line, int *i);
char* ParseLineFloat(char *line, float *f);
char* ParseLineString(char *line, char **str);
char* ParseLineTexture(char *line, char **texture);

bool ParseBool(char *str, bool *val);
bool ParseInt(char *str, int *val);
bool ParseUint(char *str, uint32_t *val);
bool ParseFloat(char *str, float *val);
bool ParseVec4(char *str, vec4s *val);
