#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdalign.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glad2/gl.h>
#include <cglm/cglm.h>

#include "pstring.h"
#include "hash.h"

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define min(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ < b_ ? a_ : b_; })
#define max(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ > b_ ? a_ : b_; })
#define clamp(mi, ma, v) ({ __typeof__(mi) mi_ = (mi); __typeof__(ma) ma_ = (ma); __typeof__(v) v_ = (v); max(mi_, min(ma_, v_)); })

#define SHADER_VERSION "#version 450 core\n"

#include "debug.h"
